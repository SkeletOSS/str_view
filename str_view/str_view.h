/** @file
@brief The `SV_Str_view` Interface

A `SV_Str_view` is a read only view of null terminated C strings. It's purpose
is to provide robust read only string construction, tokenizing, and matching.
This introduces tremendous flexibility to string handling in C programming from
low-level kernel code to application layer programming.

Read only string handling offers many safety benefits as well. All functions in
the interface are either constant or pure functions. This means that given the
same inputs, they will provide the same outputs. And because no functions can
modify the underlying string, compilers can often help use these guarantees
to produce faster code. This also means that all string view functions are
inherently safe to be used from multiple threads over the same string data.

This interface also aims to provide robust read only tokenization without any of
the concerns that come with functions like `strtok` versus `strtok_r`.
Tokenization can occur simultaneously from multiple threads and a natural
iteration abstraction is provided.

Finally, all major string matching functions are provided and run in linear
time, using constant space. There are also variants of the substring matching
algorithms that run in reverse providing the optimal time complexity for reverse
string matching. Constant space complexity is an important component of
maintaining the pure attributes of the searching function. Regardless of the
underlying string matching algorithm, no side effects occur and no auxiliary
global or static global storage is needed. */
#ifndef SV_STR_VIEW
#define SV_STR_VIEW

/* SV_ATTRIB_PURE has no side effects when given the same arguments with the
   same data, producing the same return value. A SV_Str_view points to char
   const * data which may change in between SV_Str_view calls. SV_ATTRIB_CONST
   applies only where no pointers are accessed or dereferenced. Other
   attributes provide more string safety and opportunities to optimize.
   Credit Harith on Code Review Stack Exchange. */
#if defined(__GNUC__) || defined(__clang__) || defined(__INTEL_LLVM_COMPILER)
#    ifdef __has_attribute
#        if __has_attribute(pure)
/** @brief Describes a function as having no side effects when given the same
arguments with same underlying data. Can be used when pointers are provided. */
#            define SV_ATTRIB_PURE __attribute__((pure))
#        else
/** @brief Describes a function as having no side effects when given the same
arguments with same underlying data. Can be used when pointers are provided. */
#            define SV_ATTRIB_PURE /**/
#        endif
#        if __has_attribute(pure)
/** @brief Describes a function as having no side effects and not depending on
any pointer input that could change between calls. */
#            define SV_ATTRIB_CONST __attribute__((const))
#        else
/** @brief Describes a function as having no side effects and not depending on
any pointer input that could change between calls. */
#            define SV_ATTRIB_CONST /**/
#        endif
#        if __has_attribute(null_terminated_string_arg)
/** @brief Describes a parameter as null terminated. */
#            define SV_ATTRIB_NULLTERM(...)                                    \
                __attribute__((null_terminated_string_arg(__VA_ARGS__)))
#        else
/** @brief Describes a parameter as null terminated. */
#            define SV_ATTRIB_NULLTERM(...) /**/
#        endif
#    else
/** @brief Describes a function as having no side effects when given the same
arguments with same underlying data. Can be used when pointers are provided. */
#        define SV_ATTRIB_PURE          /**/
/** @brief Describes a function as having no side effects and not depending on
any pointer input that could change between calls. */
#        define SV_ATTRIB_CONST         /**/
/** @brief Describes a parameter as null terminated. */
#        define SV_ATTRIB_NULLTERM(...) /**/
#    endif
/** @brief A helper macro to enforce only string literals for the SV_from
constructor macro. GCC and Clang allow this syntax to create more errors when
bad input is provided to the SV_from constructor.
@param[in] str_literal the input string literal */
#    define SV_STR_LITERAL(str_literal) "" str_literal ""
#else
/** @brief Describes a function as having no side effects when given the same
arguments with same underlying data. Can be used when pointers are provided. */
#    define SV_ATTRIB_PURE          /**/
/** @brief Describes a function as having no side effects and not depending on
any pointer input that could change between calls. */
#    define SV_ATTRIB_CONST         /**/
/** @brief Describes a parameter as null terminated. */
#    define SV_ATTRIB_NULLTERM(...) /**/
/** @brief MSVC does not allow strong enforcement of string literals to the
   SV_from constructor. This is a dummy wrapper for compatibility. */
#    define SV_STR_LITERAL(str_literal) str_literal
#endif /* __GNUC__ || __clang__ || __INTEL_LLVM_COMPILER */

#if defined(_MSVC_VER) || defined(_WIN32) || defined(_WIN64)
#    if defined(SV_BUILD_DLL)
/** @brief A macro to help with library linking on windows. */
#        define SV_API __declspec(dllexport)
#    elif defined(SV_CONSUME_DLL)
/** @brief A macro to help with library linking on windows. */
#        define SV_API __declspec(dllimport)
#    else
/** @brief A macro to help with library linking on windows. */
#        define SV_API /**/
#    endif
#else
/** @brief A macro to help with library linking on windows. */
#    define SV_API /**/
#endif             /* _MSVC_VER */

#include <stdbool.h>
#include <stddef.h>

/** @name Types
The types of the `SV_Str_view` interface. */
/**@{*/

/** @brief A read-only view of string data in C.

It is modeled after the C++ `std::string_view`. It consists of a pointer to char
const data and a size_t field. Therefore, the exact size of this type may be
platform dependent but it is small enough that one should use the provided
functions and pass by copy whenever possible. Avoid accessing struct fields. */
typedef struct {
    /** The read only data to which we point. */
    char const *str;
    /** The length, not including the NULL terminator position. However, it is
       possible that a view is not NULL terminated at that position. */
    size_t len;
} SV_Str_view;

/** @brief Standard three way comparison type in C.

Orders the result of a comparison between left and right hand side elements,
describing the order of the left hand side element compared to the right. */
typedef enum {
    SV_ORDER_LESSER = -1,
    SV_ORDER_EQUAL,
    SV_ORDER_GREATER,
    SV_ORDER_ERROR,
} SV_Order;

/**@}*/

/** @name Construction
A macro and functions for constructing a `SV_Str_view`. */
/**@{*/

/** @brief A macro to reduce the chance for errors in repeating oneself when
constructing an inline or const SV_Str_view. The input must be a string literal.
@param[in] str_literal a C string literal
@return a string view constructed in O(1) at compile time.
For example:

```
static SV_Str_view const prefix = SV_from("test_");
```

One can even use this in code when string literals are used rather than
saved constants to avoid errors in SV_Str_view constructions.

```
for (SV_Str_view cur = SV_token_begin(src, SV_from(" "));
    !SV_token_end(src, cur);
    cur = SV_token_next(src, cur, SV_from(" "))
{}
```

However saving the SV_Str_view in a constant may be more convenient. */
#define SV_from(str_literal)                                                   \
    ((SV_Str_view){SV_STR_LITERAL(str_literal), sizeof(str_literal) - 1})

/** @brief Constructs and returns a string view from a null terminated string.
@param[in] str a pointer to the null terminated string.
@return a constructed string view in linear time at runtime.
@warning The provided string must be null terminated. It is expected that all
strings provided to string views are null terminated. */
SV_API SV_Str_view SV_from_terminated(char const *str)
    SV_ATTRIB_NULLTERM(1) SV_ATTRIB_PURE;

/** @brief Constructs and returns a string view from a sequence of valid n bytes
or string length, whichever comes first.
@param[in] n the number of bytes to scan, at most.
@param[in] str the null terminated string to scan.
@return a string view constructed from the minimum between the length of the
input string and the provided byte count.
@warning If no `\0` character is encountered, the character found at byte `n` of
the input string is conceptually at the position of the null terminator of the
view. */
SV_API SV_Str_view SV_from_view(size_t n, char const *str)
    SV_ATTRIB_NULLTERM(2) SV_ATTRIB_PURE;

/** @brief Constructs and returns a string view from a null terminated string
broken on the first occurrence of delimiter if found or null terminator if delim
cannot be found.
@param[in] str the null terminated input string.
@param[in] delim the null terminated delimiter on which to break.
@return the constructed string view broken on the first occurrence of the
delimiter or the entire string if no delimiter is found.

This constructor will also skip the delimiter if that delimiter starts the
string. This is similar to the tokenizing function in the iteration section. */
SV_API SV_Str_view SV_from_delimiter(char const *str, char const *delim)
    SV_ATTRIB_NULLTERM(1) SV_ATTRIB_NULLTERM(2) SV_ATTRIB_PURE;

/** @brief Creates the substring from position pos for count length. The count
is the minimum value between count and `(length - pos)`.
@param[in] sv the string view.
@param[in] pos the position from which to start the substring.
@param[in] count the new string length of the substring.
@return a newly constructed string view substring. If an invalid position is
given greater than SV_Str_view length an empty view is returned positioned at
the end of SV_Str_view.
@warning The end position may or may not hold the null terminator. */
SV_API SV_Str_view SV_substr(SV_Str_view sv, size_t pos,
                             size_t count) SV_ATTRIB_PURE;

/** @brief Returns the bytes of the string pointer to, null terminator included.
@param[in] str the null terminated input string.
@return the size in bytes of str, including the null terminator. */
SV_API size_t SV_str_bytes(char const *str)
    SV_ATTRIB_NULLTERM(1) SV_ATTRIB_PURE;

/** @brief Copies the max of string bytes or input string length into a view,
whichever ends first.
@param[in] str_bytes the number of bytes to scan, at most.
@param[in] src_str the null terminated string to scan.
@return a string view constructed from the minimum between the length of the
input string and the provided byte count.
@warning If no `\0` character is encountered, the character found at byte
`str_bytes` of the input string is conceptually at the position of the null
terminator of the view. */
SV_API SV_Str_view SV_copy(size_t str_bytes, char const *src_str)
    SV_ATTRIB_NULLTERM(2) SV_ATTRIB_PURE;

/** @brief Fills the destination buffer with the minimum between destination
size and source view size, null terminating the string.
@param[in] dest_bytes the bytes available in the destination.
@param[in] dest_buf the character buffer destination.
@param[in] src the string view to copy into the buffer.
@return the number of bytes successfully written, including the null terminator.

@warning This may cut off src data if `destination bytes < source length`.
Returns how many bytes were written to the buffer. */
SV_API size_t SV_fill(size_t dest_bytes, char *dest_buf, SV_Str_view src);

/** @brief Returns a SV_Str_view of the entirety of the underlying string,
starting at the current view pointer position.
@param[in] sv the string view to extend.
@return a string view extended to include the rest of the characters before the
null terminator of the underlying string.

This guarantees that the SV_Str_view returned ends at the null terminator of the
underlying string as all strings used with SV_Str_views are assumed to be null
terminated. It is undefined behavior to provide non null terminated strings to
any SV_Str_view code. */
SV_API SV_Str_view SV_extend(SV_Str_view sv) SV_ATTRIB_PURE;

/**@}*/

/** @name Comparison
Comparing a `SV_Str_view` with another instance or a C string. */
/**@{*/

/** @brief Returns the standard C threeway comparison between cmp(lhs, rhs)
between two string views.
@param[in] lhs the string view left hand side.
@param[in] rhs the string view right hand side.
@return the order of the left hand side string view compared to the right hand
side.

Comparison is bounded by the shorter SV_Str_view length. ERR is returned if bad
input is provided such as a SV_Str_view with a NULL pointer field. */
SV_API SV_Order SV_compare(SV_Str_view lhs, SV_Str_view rhs) SV_ATTRIB_PURE;

/** @brief Returns the standard C threeway comparison between cmp(lhs, rhs)
between a SV_Str_view and a C string.
@param[in] lhs the string view that serves as the left hand side of the order.
@param[in] rhs the null terminated string to compare against.
@return the order of the left hand side compared to the right hand side:
`SV_ORDER_LESSER`, `SV_ORDER_EQUAL`, or `SV_ORDER_GREATER`.

Comparison is bounded by the shorter SV_Str_view length. An error value is
returned if bad input is provided such as a SV_Str_view with a NULL pointer
field. */
SV_API SV_Order SV_terminated_compare(SV_Str_view lhs, char const *rhs)
    SV_ATTRIB_NULLTERM(2) SV_ATTRIB_PURE;

/** @brief Returns the standard C threeway comparison between cmp(lhs, rhs)
between a SV_Str_view and a C string, bounded by n bytes of the right hand side.
If `n` is shorter than the right hand side string the `n`th byte is compared.
@param[in] lhs the string view that serves as the left hand side of the order.
@param[in] rhs the null terminated string to compare against.
@param[in] n limit bytes of the comparison if less than right hand side length.
@return the order of the left hand side compared to the right hand side:
`SV_ORDER_LESSER`, `SV_ORDER_EQUAL`, or `SV_ORDER_GREATER`.

Comparison is bounded by the shortest length between left hand side, right hand
side string length, and the byte limit n. An error value is returned if bad
input is provided such as a SV_Str_view with a NULL pointer field. */
SV_API SV_Order SV_view_compare(SV_Str_view lhs, char const *rhs, size_t n)
    SV_ATTRIB_NULLTERM(2) SV_ATTRIB_PURE;

/**@}*/

/** @name Tokenization and Iteration
Tokenize a `SV_Str_view` and use convenient iteration abstractions. */
/**@{*/

/** @brief Finds the first tokenized position in the string view given any
length delim SV_Str_view.
@param[in] src the source string view to tokenize.
@param[in] delim the delimiter that separates tokens.
@return the first token encountered in the source view or the entire view if
no token was found. Skips leading delimiters in construction. If the SV_Str_view
to be searched stores NULL than the SV_null() is returned. If delim stores NULL,
that is interpreted as a search for the null terminating character or empty
string and the size zero substring at the final position in the SV_Str_view is
returned wich may or may not be the null termiator. If no delim is found the
entire SV_Str_view is returned. */
SV_API SV_Str_view SV_token_begin(SV_Str_view src,
                                  SV_Str_view delim) SV_ATTRIB_PURE;

/** @brief Provides the status of the current tokenization for use in conditions
such as loops.
@param[in] src the source view being tokenized.
@param[in] token the current token obtained from tokenization.
@return true if no further tokens are found and position is at the end position,
meaning a call to SV_token_begin() or SV_token_next() has yielded a size 0
SV_Str_view that points at the end of the src SV_Str_view, which may or may not
be null terminated. */
SV_API bool SV_token_end(SV_Str_view src, SV_Str_view token) SV_ATTRIB_PURE;

/** @brief Advances to the next token in the remaining view separated by the
delim.
@param[in] src the source string view being tokenized.
@param[in] token the current token obtained from tokenizing.
@param[in] delim the delimeter that separates tokens.
@return the string view of the next available token. Repeating delimiter
patterns will be skipped until the next token or end of string is found. If
SV_Str_view stores NULL the SV_null() placeholder is returned. If delim stores
NULL the end position of the SV_Str_view is returned which may or may not be the
null terminator. The token is bounded by the length of the view between two
delimiters or the length from a delimiter to the end of src, whichever comes
first. */
SV_API SV_Str_view SV_token_next(SV_Str_view src, SV_Str_view token,
                                 SV_Str_view delim) SV_ATTRIB_PURE;

/** Obtains the last token in a string in preparation for reverse tokenized
iteration.
@param[in] src the source view to tokenize in reverse.
@param[in] delim the delimiter that separates tokens.
@return the last token in the source view or the entire view if no token was
found. Any delimiters that end the string are skipped, as in the forward
version. If src is NULL SV_null is returned. If delim is null the entire src
view is returned.
@note Though the SV_Str_view is tokenized in reverse, the token view returned
will start at the first character and be the length of the token found. */
SV_API SV_Str_view SV_token_reverse_begin(SV_Str_view src,
                                          SV_Str_view delim) SV_ATTRIB_PURE;

/** @brief Provides the status of the current tokenization for use in conditions
such as loops.
@param[in] src the source view being tokenized.
@param[in] token the current token obtained from tokenization.
@return true if no further tokens are found and position is at the start
position, meaning a call to SV_token_reverse_begin() or SV_token_reverse_next()
has yielded a size 0 SV_Str_view that points at the start of the src
SV_Str_view. */
SV_API bool SV_token_reverse_end(SV_Str_view src,
                                 SV_Str_view token) SV_ATTRIB_PURE;

/** @brief Advances the token in src to the next token between two delimiters
provided by delim.
@param[in] src the source string view being reverse tokenized.
@param[in] token the current token obtained from tokenization.
@param[in] delim the delimiter that separates tokens.
@return the string view of the next available token. Repeating delimiters are
skipped until the next token is found. If no further tokens can be found an
empty SV_Str_view is returned with its pointer set to the start of the src
string being iterated through.
@note A multi-character delimiter may yield different tokens in reverse than in
the forward direction when partial matches occur and some portion of the
delimiter is in a token. This is because the string is now being parsed from
right to left. However, the token returned starts at the first character and is
read from left to right between two delimiters as in the forward version. */
SV_API SV_Str_view SV_token_reverse_next(SV_Str_view src, SV_Str_view token,
                                         SV_Str_view delim) SV_ATTRIB_PURE;

/** @brief Returns a read only pointer to the beginning of the string view,
the first valid character in the view.
@param[in] sv the input string view.
@return if the view stores NULL, the placeholder SV_null() is returned. */
SV_API char const *SV_begin(SV_Str_view sv) SV_ATTRIB_PURE;

/** @brief Returns the reverse iterator beginning, the last character of the
current view.
@param[in] sv the input string view.
@return if the view is null SV_null() is returned. If the view is sized zero
with a valid pointer that pointer in the view is returned. */
SV_API char const *SV_reverse_begin(SV_Str_view sv) SV_ATTRIB_PURE;

/** @brief Advances the null terminated string pointer from its previous
position. If NULL is provided SV_null() is returned.
@param[in] c the pointer to a null terminated string iterator.
@return the next character in the string or the end iterator if no characters
remain. */
SV_API char const *SV_next(char const *c) SV_ATTRIB_NULLTERM(1) SV_ATTRIB_PURE;

/** @brief Advances the iterator to the next character in the SV_Str_view being
iterated through in reverse.
@param[in] c the pointer to the null terminated string iterator.
@warning It is undefined behavior to change the SV_Str_view one is iterating
through during iteration.
If the char pointer is null, SV_null() is returned. */
SV_API char const *SV_reverse_next(char const *c) SV_ATTRIB_PURE;

/** @brief Returns a read only pointer to the end of the string view.
@param[in] sv the view being iterated over.
@return the character pointer to the end of this iteration. This may or may not
be a null terminated character depending on the view. If the view stores NULL,
the placeholder SV_null() is returned. */
SV_API char const *SV_end(SV_Str_view sv) SV_ATTRIB_PURE;

/** @brief The ending position of a reverse iteration.
@param[in] sv the view being iterated over in reverse.
@return the character position at the reverse end of this iteration.
@warning It is undefined behavior to access or use rend. It is undefined
behavior to pass in any SV_Str_view not being iterated through as started with
reverse_begin. */
SV_API char const *SV_reverse_end(SV_Str_view sv) SV_ATTRIB_PURE;

/**@}*/

/** @name String Matching
Search a `SV_Str_view` for various types of substrings and character matches. */
/**@{*/

/** @brief Searches for needle in haystack starting from pos.
@param[in] haystack the string view to search.
@param[in] pos the position from which to start the search.
@param[in] needle the substring to match within haystack.
@return the index of the first character of the match. If the needle is larger
than the haystack, or position is greater than haystack length, then haystack
length (npos) is returned. */
SV_API size_t SV_find(SV_Str_view haystack, size_t pos,
                      SV_Str_view needle) SV_ATTRIB_PURE;

/** @brief Searches for the last occurrence of needle in haystack starting from
pos from right to left.
@param[in] haystack the string view to search.
@param[in] pos the position from which to start the search.
@param[in] needle the substring to match within haystack.
@return if found the starting position of the string is returned, the same as
find. If not found haystack size is returned. The only difference from find is
the search direction. If needle is larger than haystack, haystack length is
returned.

If the position is larger than the haystack, the entire haystack is searched. */
SV_API size_t SV_reverse_find(SV_Str_view haystack, size_t pos,
                              SV_Str_view needle) SV_ATTRIB_PURE;

/** @brief Tests membership of needle in haystack.
@param[in] haystack the source string view to search.
@param[in] needle the substring to match within haystack.
@return true if the needle is found in the haystack, false otherwise. */
SV_API bool SV_contains(SV_Str_view haystack,
                        SV_Str_view needle) SV_ATTRIB_PURE;

/** @brief Search for a substring within a source string view.
@param[in] haystack the source string view to search.
@param[in] needle the substring to match within haystack.
@return a view of the needle found in haystack at the first found position. If
the needle cannot be found the empty view at the haystack length position is
returned. This may or may not be null terminated at that position. If needle is
greater than haystack length an empty view at the end of haystack is returned.
If haystack is NULL, SV_null is returned. */
SV_API SV_Str_view SV_match(SV_Str_view haystack,
                            SV_Str_view needle) SV_ATTRIB_PURE;

/** @brief Search for a substring within a source string in reverse.
@param[in] haystack the source string view to reverse search.
@param[in] needle the substring to match within haystack.
@return a view of the needle found in haystack at the last found position. If
the needle cannot be found the empty view at the haystack length position is
returned. This may or may not be null terminated at that position. If needle is
greater than haystack length an empty view at haystack size is returned. If
haystack is NULL, SV_null is returned (modeled after strstr). */
SV_API SV_Str_view SV_reverse_match(SV_Str_view haystack,
                                    SV_Str_view needle) SV_ATTRIB_PURE;

/** @brief Confirms the presence of a prefix within a string view.
@param[in] sv the string view to search.
@param[in] prefix the substring prefix to match with the input view.
@return true if a prefix shorter than or equal in length to the SV_Str_view is
present, false otherwise. */
SV_API bool SV_starts_with(SV_Str_view sv, SV_Str_view prefix) SV_ATTRIB_PURE;

/** Removes the minimum between SV_Str_view length and n from the start of the
SV_Str_view.
@param[in] sv the input string view.
@param[in] n the bytes to remove from the start of the input view.
@return the new view with the requested bytes removed from the start.

It is safe to provide n larger than SV_Str_view size as that will result in a
size 0 view to the end of the current view which may or may not be the null
terminator. */
SV_API SV_Str_view SV_remove_prefix(SV_Str_view sv, size_t n) SV_ATTRIB_PURE;

/** @brief Confirms the presence of a suffix within a string view.
@param[in] sv the string view to search.
@param[in] suffix the substring suffix to match with the input view.
@return true if a suffix shorter than or equal in length to the SV_Str_view is
present, false otherwise. */
SV_API bool SV_ends_with(SV_Str_view sv, SV_Str_view suffix) SV_ATTRIB_PURE;

/** Removes the minimum between SV_Str_view length and n from the end of the
SV_Str_view.
@param[in] sv the input string view.
@param[in] n the bytes to remove from the end of the input view.
@return the new view with the requested bytes removed from the end.

It is safe to provide n larger than SV_Str_view size as that will result in a
size 0 view to the end of the current view which may or may not be the null
terminator. */
SV_API SV_Str_view SV_remove_suffix(SV_Str_view sv, size_t n) SV_ATTRIB_PURE;

/** Finds the first position of an occurrence of any character in set.
@param[in] haystack the input view to search.
@param[in] set the set of characters to search for in the haystack. Each
character in the input set is considered a valid match if encountered in
haystack.
@return the position of the first occurrence of any character in the input set
provided. If no occurrence is found haystack size is returned. An empty set
(NULL) is valid, returning npos. An empty haystack returns 0. */
SV_API size_t SV_find_first_of(SV_Str_view haystack,
                               SV_Str_view set) SV_ATTRIB_PURE;

/** Finds the first position at which no characters in set can be found.
@param[in] haystack the input view to search.
@param[in] set the set of characters banned in the search for of haystack. Each
character in the input set is considered a valid match if encountered in
haystack.
@return the position of the first occurrence of any character NOT in the input
set provided. If only characters in the set are encountered haystack size is
returned. An empty set (NULL) is valid, returning npos. An empty haystack
returns 0. */
SV_API size_t SV_find_first_not_of(SV_Str_view haystack,
                                   SV_Str_view set) SV_ATTRIB_PURE;

/** Finds the last position of an occurrence of any character in set.
@param[in] haystack the input view to search.
@param[in] set the set of characters to search for in the haystack. Each
character in the input set is considered a valid match if encountered in
haystack.
@return the position of the last occurrence of any character in the input set
provided. If no occurrence is found haystack size is returned. An empty set
(NULL) is valid, returning npos. An empty haystack returns 0. */
SV_API size_t SV_find_last_of(SV_Str_view haystack,
                              SV_Str_view set) SV_ATTRIB_PURE;

/** Finds the last position at which no characters in set can be found.
@param[in] haystack the input view to search.
@param[in] set the set of characters banned in the search for of haystack. Each
character in the input set is considered a valid match if encountered in
haystack.
@return the position of the last occurrence of any character NOT in the input
set provided. If only characters in the set are encountered haystack size is
returned. An empty set (NULL) is valid, returning npos. An empty haystack
returns 0. */
SV_API size_t SV_find_last_not_of(SV_Str_view haystack,
                                  SV_Str_view set) SV_ATTRIB_PURE;

/**@}*/

/** @name State
Obtain current state of an `SV_Str_view` and C strings. */
/**@{*/

/** @brief Returns the minimum between the string size vs n bytes.
@param[in] str the null terminated input string.
@param[in] n the limiting byte count to compare.
@return the minimum between the length of the string and n. */
SV_API size_t SV_min_len(char const *str, size_t n)
    SV_ATTRIB_NULLTERM(1) SV_ATTRIB_PURE;

/** @brief A sentinel empty string. Safely dereference to view a null
terminator. This may be returned from various functions when bad input is given
such as NULL pointers as the underlying SV_Str_view string pointer.
@return a read only character pointer that points to a null terminator and can
be safely dereferenced. */
SV_API char const *SV_null(void) SV_ATTRIB_PURE;

/** @brief The end of a SV_Str_view guaranteed to be greater than or equal to
size.
@param[in] sv the string view.
@return the end position of the string as an index.

This value may be used for the idiomatic check for most string searching
function return values when something is not found. If a size is returned from a
searching function it is possible to check it against this value. */
SV_API size_t SV_npos(SV_Str_view sv) SV_ATTRIB_CONST;

/** @brief Returns true if the provided SV_Str_view is empty, false otherwise.
@param[in] sv the string view to check.
@return true if empty otherwise false.

This is a useful function to check for SV_Str_view searches that yield an empty
view at the end of a SV_Str_view when an element cannot be found. */
SV_API bool SV_is_empty(SV_Str_view sv) SV_ATTRIB_CONST;

/** @brief Returns the length of the SV_Str_view in O(1) time.
@param[in] sv the string view to check.
@return the length of the string view, not including the null terminator byte.

The position at SV_Str_view size is interpreted as the null terminator and not
counted toward length of a SV_Str_view. */
SV_API size_t SV_len(SV_Str_view sv) SV_ATTRIB_CONST;

/** Returns the bytes of SV_Str_view including null terminator.
@param[in] sv the string view to check.
@return the size in bytes of the view including the null terminator position
character.
@note String views may not actually be null terminated but the position at
SV_Str_view[SV_Str_view.len] is interpreted as the null terminator and thus
counts towards the byte count. */
SV_API size_t SV_bytes(SV_Str_view sv) SV_ATTRIB_CONST;

/** @brief Returns the character pointer at the minimum between the indicated
position and the end of the string view.
@param[in] sv the string view input.
@param[in] i the index within range of `[0, string view length - 1)`.
@return a pointer to the character at the designated index. If NULL is stored by
the SV_Str_view then SV_null() is returned. */
SV_API char const *SV_pointer(SV_Str_view sv, size_t i) SV_ATTRIB_PURE;

/** @brief Obtain a character at a position in the string view.
@param[in] sv the input string view.
@param[in] i the index within `[0, string view length - 1)`
@return the character in the string at position i with bounds checking. If i is
greater than or equal to the size of SV_Str_view the null terminator character
is returned. */
SV_API char SV_at(SV_Str_view sv, size_t i) SV_ATTRIB_PURE;

/** @brief Obtain the character at the first position of SV_Str_view.
@param[in] sv the input string view.
@return the first character in the string view. An empty SV_Str_view or NULL
pointer is valid and will return '\0'. */
SV_API char SV_front(SV_Str_view sv) SV_ATTRIB_PURE;

/** @brief Obtain the character at the last position of SV_Str_view.
@param[in] sv the input string view.
@return the last character in the string view. An empty SV_Str_view or NULL
pointer is valid and will return '\0'. */
SV_API char SV_back(SV_Str_view sv) SV_ATTRIB_PURE;

/**@}*/

#endif /* SV_STR_VIEW */
