/* Author: Alexander G. Lopez
   ==========================
   This file implements the SV_Str_view interface as an interpretation of C++
   string_view type. There are some minor differences and C flavor thrown
   in. Additionally, there is a provided reimplementation of the Two-Way
   String-Searching algorithm, similar to glibc. */
#include "str_view.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Clang and GCC support static array parameter declarations while
   MSVC does not. This is how to solve the differing declaration
   signature requirements. */
#if defined(__GNUC__) || defined(__clang__) || defined(__INTEL_LLVM_COMPILER)
/* A static array parameter declaration helper. Function parameters
   may specify an array of a type of at least SIZE elements large,
   allowing compiler optimizations and safety errors. Specify
   a parameter such as `void func(int size, ARR_GEQ(arr, size))`. */
#    define ARR_GEQ(str, size) str[static(size)]
/* A static array parameter declaration helper. Function parameters
   may specify an array of a type of at least SIZE elements large,
   allowing compiler optimizations and safety errors. Specify
   a parameter such as `void func(int size, int ARR_GEQ(arr,size))`.
   This declarations adds the additional constraint that the pointer
   to the beginning of the array of types will not move. */
#    define ARR_CONST_GEQ(str, size) str[static const(size)]
#else
/* Dummy macro for MSVC compatibility. Specifies a function parameter shall
   have at least one element. Compiler warnings may differ from GCC/Clang. */
#    define ARR_GEQ(str, size) *str
/* Dummy macro for MSVC compatibility. Specifies a function parameter shall
   have at least one element. MSVC does not allow specification of a const
   pointer to the beginning of an array function parameter when using array
   size parameter syntax. Compiler warnings may differ from GCC/Clang. */
#    define ARR_CONST_GEQ(str, size) *const str
#endif

/* ========================   Type Definitions   =========================== */

/* Return the factorization step of two-way search in pre-compute phase. */
struct Factorization {
    /* Position in the needle at which (local period = period). */
    ptrdiff_t critical_position;
    /* A distance in the needle such that two letters always coincide. */
    ptrdiff_t period_distance;
};

/* Avoid giving the user a chance to dereference null as much as possible
   by returning this for various edge cases when it makes sense to communicate
   empty, null, invalid, not found etc. Used on cases by case basis.
   The function interfaces protect us from null pointers but not always. */
static SV_Str_view const nil = {
    .str = "",
    .len = 0,
};

/* =========================   Prototypes   =============================== */

static size_t after_find(SV_Str_view, SV_Str_view);
static size_t before_reverse_find(SV_Str_view, SV_Str_view);
static size_t min(size_t, size_t);
static SV_Order char_compare(char, char);
static ptrdiff_t signed_max(ptrdiff_t, ptrdiff_t);

/* Once the user facing API has verified the lengths of strings provided to
   views as inputs, internal code can take advantage of compiler optimizations
   by assuming the strings are GREATER than or EQUAL TO certain lengths
   allowing for processing by larger units than 1 in compiled code. */

static size_t position_memoized(ptrdiff_t haystack_size,
                                char const ARR_GEQ(, haystack_size),
                                ptrdiff_t needle_size,
                                char const ARR_GEQ(, needle_size), ptrdiff_t,
                                ptrdiff_t);
static size_t position_normal(ptrdiff_t haystack_size,
                              char const ARR_GEQ(, haystack_size),
                              ptrdiff_t needle_size,
                              char const ARR_GEQ(, needle_size), ptrdiff_t,
                              ptrdiff_t);
static size_t reverse_position_memoized(ptrdiff_t haystack_size,
                                        char const ARR_GEQ(, haystack_size),
                                        ptrdiff_t needle_size,
                                        char const ARR_GEQ(, needle_size),
                                        ptrdiff_t, ptrdiff_t);
static size_t reverse_position_normal(ptrdiff_t haystack_size,
                                      char const ARR_GEQ(, haystack_size),
                                      ptrdiff_t needle_size,
                                      char const ARR_GEQ(, needle_size),
                                      ptrdiff_t, ptrdiff_t);
static size_t two_way_match(ptrdiff_t haystack_size,
                            char const ARR_GEQ(, haystack_size),
                            ptrdiff_t needle_size,
                            char const ARR_GEQ(, needle_size));
static size_t two_way_reverse_match(ptrdiff_t haystack_size,
                                    char const ARR_GEQ(, haystack_size),
                                    ptrdiff_t needle_size,
                                    char const ARR_GEQ(, needle_size));
static struct Factorization maximal_suffix(ptrdiff_t needle_size,
                                           char const ARR_GEQ(, needle_size));
static struct Factorization
maximal_suffix_reverse(ptrdiff_t needle_size,
                       char const ARR_GEQ(, needle_size));
static struct Factorization
reverse_maximal_suffix(ptrdiff_t needle_size,
                       char const ARR_GEQ(, needle_size));
static struct Factorization
reverse_maximal_suffix_reverse(ptrdiff_t needle_size,
                               char const ARR_GEQ(, needle_size));
static size_t two_byte_view_match(size_t haystack_size,
                                  unsigned char const ARR_GEQ(, haystack_size),
                                  size_t n_size,
                                  unsigned char const ARR_GEQ(, n_size));
static size_t three_byte_view_match(size_t size,
                                    unsigned char const ARR_GEQ(, size),
                                    size_t n_size,
                                    unsigned char const ARR_GEQ(, n_size));
static size_t four_byte_view_match(size_t size,
                                   unsigned char const ARR_GEQ(, size),
                                   size_t n_size,
                                   unsigned char const ARR_GEQ(, n_size));
static size_t view_complimentary_substring_length(
    size_t str_size, char const ARR_GEQ(, str_size), size_t set_size,
    char const ARR_GEQ(, set_size));
static size_t view_substring_length(size_t str_size,
                                    char const ARR_GEQ(, str_size),
                                    size_t set_size,
                                    char const ARR_GEQ(, set_size));
static size_t view_match(ptrdiff_t haystack_size,
                         char const ARR_GEQ(, haystack_size),
                         ptrdiff_t needle_size,
                         char const ARR_GEQ(, needle_size));
static size_t view_match_char(size_t n, char const ARR_GEQ(, n), char);
static size_t reverse_view_match_char(size_t n, char const ARR_GEQ(, n), char);
static size_t reverse_view_match(ptrdiff_t haystack_size,
                                 char const ARR_GEQ(, haystack_size),
                                 ptrdiff_t needle_size,
                                 char const ARR_GEQ(, needle_size));
static size_t
reverse_two_byte_view_match(size_t size, unsigned char const ARR_GEQ(, size),
                            size_t n_size,
                            unsigned char const ARR_GEQ(, n_size));
static size_t
reverse_three_byte_view_match(size_t size, unsigned char const ARR_GEQ(, size),
                              size_t n_size,
                              unsigned char const ARR_GEQ(, n_size));
static size_t
reverse_four_byte_view_match(size_t size, unsigned char const ARR_GEQ(, size),
                             size_t n_size,
                             unsigned char const ARR_GEQ(, n_size));

/* ===================   Interface Implementation   ====================== */

SV_Str_view
SV_from_terminated(char const *const str) {
    if (!str) {
        return nil;
    }
    return (SV_Str_view){
        .str = str,
        .len = strlen(str),
    };
}

SV_Str_view
SV_from_view(size_t n, char const *const str) {
    if (!str) {
        return nil;
    }
    return (SV_Str_view){
        .str = str,
        .len = strnlen(str, n),
    };
}

SV_Str_view
SV_from_delimiter(char const *const str, char const *const delim) {
    if (!str) {
        return nil;
    }
    if (!delim) {
        return (SV_Str_view){
            .str = str,
            .len = strlen(str),
        };
    }
    return SV_token_begin(
        (SV_Str_view){
            .str = str,
            .len = strlen(str),
        },
        (SV_Str_view){
            .str = delim,
            .len = strlen(delim),
        });
}

SV_Str_view
SV_copy(size_t const str_bytes, char const *const src_str) {
    return SV_from_view(str_bytes, src_str);
}

size_t
SV_fill(size_t const dest_bytes, char *const dest_buf, SV_Str_view const src) {
    if (!dest_buf || !dest_bytes || !src.str || !src.len) {
        return 0;
    }
    size_t const bytes = min(dest_bytes, SV_bytes(src));
    memmove(dest_buf, src.str, bytes);
    dest_buf[bytes - 1] = '\0';
    return bytes;
}

bool
SV_is_empty(SV_Str_view const sv) {
    return !sv.len;
}

size_t
SV_len(SV_Str_view const sv) {
    return sv.len;
}

size_t
SV_bytes(SV_Str_view const sv) {
    return sv.len + 1;
}

size_t
SV_str_bytes(char const *const str) {
    if (!str) {
        return 0;
    }
    return strlen(str) + 1;
}

size_t
SV_min_len(char const *const str, size_t n) {
    return strnlen(str, n);
}

char
SV_at(SV_Str_view const sv, size_t const i) {
    if (i >= sv.len) {
        return *nil.str;
    }
    return sv.str[i];
}

char const *
SV_null(void) {
    return nil.str;
}

SV_Order
SV_compare(SV_Str_view const lhs, SV_Str_view const rhs) {
    if (!lhs.str || !rhs.str) {
        return SV_ORDER_ERROR;
    }
    size_t const size = min(lhs.len, rhs.len);
    size_t i = 0;
    for (; i < size && lhs.str[i] == rhs.str[i]; ++i) {}
    if (i == lhs.len && i == rhs.len) {
        return SV_ORDER_EQUAL;
    }
    if (i < lhs.len && i < rhs.len) {
        return (uint8_t)lhs.str[i] < (uint8_t)rhs.str[i] ? SV_ORDER_LESSER
                                                         : SV_ORDER_GREATER;
    }
    return (i < lhs.len) ? SV_ORDER_GREATER : SV_ORDER_LESSER;
}

SV_Order
SV_terminated_compare(SV_Str_view const lhs, char const *const rhs) {
    if (!lhs.str || !rhs) {
        return SV_ORDER_ERROR;
    }
    size_t const size = lhs.len;
    size_t i = 0;
    for (; i < size && rhs[i] && lhs.str[i] == rhs[i]; ++i) {}
    if (i == lhs.len && !rhs[i]) {
        return SV_ORDER_EQUAL;
    }
    if (i < lhs.len && rhs[i]) {
        return (uint8_t)lhs.str[i] < (uint8_t)rhs[i] ? SV_ORDER_LESSER
                                                     : SV_ORDER_GREATER;
    }
    return (i < lhs.len) ? SV_ORDER_GREATER : SV_ORDER_LESSER;
}

SV_Order
SV_view_compare(SV_Str_view const lhs, char const *const rhs, size_t const n) {
    if (!lhs.str || !rhs) {
        return SV_ORDER_ERROR;
    }
    size_t const size = min(lhs.len, n);
    size_t i = 0;
    for (; i < size && rhs[i] && lhs.str[i] == rhs[i]; ++i) {}
    if (i == lhs.len && size == n) {
        return SV_ORDER_EQUAL;
    }
    /* strncmp compares the first at most n bytes inclusive */
    if (i < lhs.len && size <= n) {
        return (uint8_t)lhs.str[i] < (uint8_t)rhs[i] ? SV_ORDER_LESSER
                                                     : SV_ORDER_GREATER;
    }
    return (i < lhs.len) ? SV_ORDER_GREATER : SV_ORDER_LESSER;
}

char
SV_front(SV_Str_view const sv) {
    if (!sv.str || !sv.len) {
        return *nil.str;
    }
    return *sv.str;
}

char
SV_back(SV_Str_view const sv) {
    if (!sv.str || !sv.len) {
        return *nil.str;
    }
    return sv.str[sv.len - 1];
}

char const *
SV_begin(SV_Str_view const sv) {
    if (!sv.str) {
        return nil.str;
    }
    return sv.str;
}

char const *
SV_end(SV_Str_view const sv) {
    if (!sv.str || sv.str == nil.str) {
        return nil.str;
    }
    return sv.str + sv.len;
}

char const *
SV_next(char const *c) {
    if (!c) {
        return nil.str;
    }
    return ++c;
}

char const *
SV_reverse_begin(SV_Str_view const sv) {
    if (!sv.str) {
        return nil.str;
    }
    if (!sv.len) {
        return sv.str;
    }
    return sv.str + sv.len - 1;
}

char const *
SV_reverse_end(SV_Str_view const sv) {
    if (!sv.str || sv.str == nil.str) {
        return nil.str;
    }
    if (!sv.len) {
        return sv.str;
    }
    return sv.str - 1;
}

char const *
SV_reverse_next(char const *c) {
    if (!c) {
        return nil.str;
    }
    return --c;
}

char const *
SV_pointer(SV_Str_view const sv, size_t const i) {
    if (!sv.str) {
        return nil.str;
    }
    if (i > sv.len) {
        return SV_end(sv);
    }
    return sv.str + i;
}

SV_Str_view
SV_token_begin(SV_Str_view src, SV_Str_view const delim) {
    if (!src.str) {
        return nil;
    }
    if (!delim.str) {
        return (SV_Str_view){.str = src.str + src.len, 0};
    }
    char const *const begin = src.str;
    size_t const sv_not = after_find(src, delim);
    src.str += sv_not;
    if (begin + src.len == src.str) {
        return (SV_Str_view){
            .str = src.str,
            .len = 0,
        };
    }
    src.len -= sv_not;
    return (SV_Str_view){
        .str = src.str,
        .len = SV_find(src, 0, delim),
    };
}

bool
SV_token_end(SV_Str_view const src, SV_Str_view const token) {
    return !token.len || token.str >= (src.str + src.len);
}

SV_Str_view
SV_token_next(SV_Str_view const src, SV_Str_view const token,
              SV_Str_view const delim) {
    if (!token.str) {
        return nil;
    }
    if (!delim.str || !token.str || !token.str[token.len]) {
        return (SV_Str_view){
            .str = &token.str[token.len],
            .len = 0,
        };
    }
    SV_Str_view next = {
        .str = &token.str[token.len] + delim.len,
    };
    if (next.str >= &src.str[src.len]) {
        return (SV_Str_view){
            .str = &src.str[src.len],
            .len = 0,
        };
    }
    next.len = &src.str[src.len] - next.str;
    /* There is a cheap easy way to skip repeating delimiters before the
       next search that should be faster than string comparison. */
    size_t const after_delim = after_find(next, delim);
    next.str += after_delim;
    next.len -= after_delim;
    if (next.str >= &src.str[src.len]) {
        return (SV_Str_view){
            .str = &src.str[src.len],
            .len = 0,
        };
    }
    size_t const found = view_match((ptrdiff_t)next.len, next.str,
                                    (ptrdiff_t)delim.len, delim.str);
    return (SV_Str_view){
        .str = next.str,
        .len = found,
    };
}

SV_Str_view
SV_token_reverse_begin(SV_Str_view src, SV_Str_view const delim) {
    if (!src.str) {
        return nil;
    }
    if (!delim.str) {
        return (SV_Str_view){
            .str = src.str + src.len,
            0,
        };
    }
    size_t before_delim = before_reverse_find(src, delim);
    src.len = min(src.len, before_delim + 1);
    size_t start = SV_reverse_find(src, src.len, delim);
    if (start == src.len) {
        return src;
    }
    start += delim.len;
    return (SV_Str_view){
        .str = src.str + start,
        .len = before_delim - start + 1,
    };
}

SV_Str_view
SV_token_reverse_next(SV_Str_view const src, SV_Str_view const token,
                      SV_Str_view const delim) {
    if (!token.str) {
        return nil;
    }
    if (!token.len | !delim.str || token.str == src.str
        || token.str - delim.len <= src.str) {
        return (SV_Str_view){
            .str = src.str,
            .len = 0,
        };
    }
    SV_Str_view const shorter = {
        .str = src.str,
        .len = (token.str - delim.len) - src.str,
    };
    /* Same as in the forward version, this method is a quick way to skip
       any number of repeating delimiters before starting the next search
       for a delimiter before a token. */
    size_t const before_delim = before_reverse_find(shorter, delim);
    if (before_delim == shorter.len) {
        return shorter;
    }
    size_t start = reverse_view_match((ptrdiff_t)before_delim, shorter.str,
                                      (ptrdiff_t)delim.len, delim.str);
    if (start == before_delim) {
        return (SV_Str_view){
            .str = shorter.str,
            .len = before_delim + 1,
        };
    }
    start += delim.len;
    return (SV_Str_view){
        .str = src.str + start,
        .len = before_delim - start + 1,
    };
}

bool
SV_token_reverse_end(SV_Str_view const src, SV_Str_view const token) {
    return !token.len && token.str == src.str;
}

SV_Str_view
SV_extend(SV_Str_view sv) {
    if (!sv.str) {
        return nil;
    }
    char const *i = sv.str;
    while (*i++) {}
    sv.len = i - sv.str - 1;
    return sv;
}

bool
SV_starts_with(SV_Str_view const sv, SV_Str_view const prefix) {
    if (prefix.len > sv.len) {
        return false;
    }
    return SV_compare(SV_substr(sv, 0, prefix.len), prefix) == SV_ORDER_EQUAL;
}

SV_Str_view
SV_remove_prefix(SV_Str_view const sv, size_t const n) {
    size_t const remove = min(sv.len, n);
    return (SV_Str_view){
        .str = sv.str + remove,
        .len = sv.len - remove,
    };
}

bool
SV_ends_with(SV_Str_view const sv, SV_Str_view const suffix) {
    if (suffix.len > sv.len) {
        return false;
    }
    return SV_compare(SV_substr(sv, sv.len - suffix.len, suffix.len), suffix)
        == SV_ORDER_EQUAL;
}

SV_Str_view
SV_remove_suffix(SV_Str_view const sv, size_t const n) {
    if (!sv.str) {
        return nil;
    }
    return (SV_Str_view){
        .str = sv.str,
        .len = sv.len - min(sv.len, n),
    };
}

SV_Str_view
SV_substr(SV_Str_view const sv, size_t const pos, size_t const count) {
    if (pos > sv.len) {
        return (SV_Str_view){
            .str = sv.str + sv.len,
            .len = 0,
        };
    }
    return (SV_Str_view){
        .str = sv.str + pos,
        .len = min(count, sv.len - pos),
    };
}

bool
SV_contains(SV_Str_view const haystack, SV_Str_view const needle) {
    if (needle.len > haystack.len) {
        return false;
    }
    if (SV_is_empty(haystack)) {
        return false;
    }
    if (SV_is_empty(needle)) {
        return true;
    }
    return haystack.len
        != view_match((ptrdiff_t)haystack.len, haystack.str,
                      (ptrdiff_t)needle.len, needle.str);
}

SV_Str_view
SV_match(SV_Str_view const haystack, SV_Str_view const needle) {
    if (!haystack.str || !needle.str) {
        return nil;
    }
    if (needle.len > haystack.len || SV_is_empty(haystack)
        || SV_is_empty(needle)) {
        return (SV_Str_view){
            .str = haystack.str + haystack.len,
            .len = 0,
        };
    }
    size_t const found = view_match((ptrdiff_t)haystack.len, haystack.str,
                                    (ptrdiff_t)needle.len, needle.str);
    if (found == haystack.len) {
        return (SV_Str_view){
            .str = haystack.str + haystack.len,
            .len = 0,
        };
    }
    return (SV_Str_view){
        .str = haystack.str + found,
        .len = needle.len,
    };
}

SV_Str_view
SV_reverse_match(SV_Str_view const haystack, SV_Str_view const needle) {
    if (!haystack.str) {
        return nil;
    }
    if (SV_is_empty(haystack) || SV_is_empty(needle)) {
        return (SV_Str_view){
            .str = haystack.str + haystack.len,
            .len = 0,
        };
    }
    size_t const found
        = reverse_view_match((ptrdiff_t)haystack.len, haystack.str,
                             (ptrdiff_t)needle.len, needle.str);
    if (found == haystack.len) {
        return (SV_Str_view){.str = haystack.str + haystack.len, .len = 0};
    }
    return (SV_Str_view){
        .str = haystack.str + found,
        .len = needle.len,
    };
}

size_t
SV_find(SV_Str_view const haystack, size_t const pos,
        SV_Str_view const needle) {
    if (needle.len > haystack.len || pos > haystack.len) {
        return haystack.len;
    }
    return pos
         + view_match((ptrdiff_t)(haystack.len - pos), haystack.str + pos,
                      (ptrdiff_t)needle.len, needle.str);
}

size_t
SV_reverse_find(SV_Str_view const h, size_t pos, SV_Str_view const n) {
    if (!h.len || n.len > h.len) {
        return h.len;
    }
    if (pos >= h.len) {
        pos = h.len - 1;
    }
    size_t const found = reverse_view_match((ptrdiff_t)pos + 1, h.str,
                                            (ptrdiff_t)n.len, n.str);
    return found == pos + 1 ? h.len : found;
}

size_t
SV_find_first_of(SV_Str_view const haystack, SV_Str_view const set) {
    if (!haystack.str || !haystack.len) {
        return 0;
    }
    if (!set.str || !set.len) {
        return haystack.len;
    }
    return view_complimentary_substring_length(haystack.len, haystack.str,
                                               set.len, set.str);
}

size_t
SV_find_last_of(SV_Str_view const haystack, SV_Str_view const set) {
    if (!haystack.str || !haystack.len) {
        return 0;
    }
    if (!set.str || !set.len) {
        return haystack.len;
    }
    /* It may be tempting to go right to left but consider if that really
       would be reliably faster across every possible string one encounters.
       The last occurrence of a set char could be anywhere in the string. */
    size_t last_pos = haystack.len;
    for (size_t in = 0, prev = 0;
         (in += view_substring_length(haystack.len - in, haystack.str + in,
                                      set.len, set.str))
         != haystack.len;
         ++in, prev = in) {
        if (in != prev) {
            last_pos = in - 1;
        }
    }
    return last_pos;
}

size_t
SV_find_first_not_of(SV_Str_view const haystack, SV_Str_view const set) {
    if (!haystack.str || !haystack.len) {
        return 0;
    }
    if (!set.str || !set.len) {
        return 0;
    }
    return view_substring_length(haystack.len, haystack.str, set.len, set.str);
}

size_t
SV_find_last_not_of(SV_Str_view const haystack, SV_Str_view const set) {
    if (!haystack.str || !haystack.len) {
        return 0;
    }
    if (!set.str || !set.len) {
        return haystack.len - 1;
    }
    size_t last_pos = haystack.len;
    for (size_t in = 0, prev = 0;
         (in += view_substring_length(haystack.len - in, haystack.str + in,
                                      set.len, set.str))
         != haystack.len;
         ++in, prev = in) {
        if (in != prev) {
            last_pos = in;
        }
    }
    return last_pos;
}

size_t
SV_npos(SV_Str_view const sv) {
    return sv.len;
}

/* ======================   Static Helpers    ============================= */

static size_t
after_find(SV_Str_view const haystack, SV_Str_view const needle) {
    if (needle.len > haystack.len) {
        return 0;
    }
    size_t delim_i = 0;
    size_t i = 0;
    while (i < haystack.len && needle.str[delim_i] == haystack.str[i]) {
        delim_i = (delim_i + 1) % needle.len;
        ++i;
    }
    /* Also reset to the last mismatch found. If some of the delimeter matched
       but then the string changed into a mismatch go back to get characters
       that are partially in the delimeter. */
    return i - delim_i;
}

static size_t
before_reverse_find(SV_Str_view const haystack, SV_Str_view const needle) {
    if (needle.len > haystack.len || !needle.len || !haystack.len) {
        return haystack.len;
    }
    size_t delim_i = 0;
    size_t i = 0;
    while (i < haystack.len
           && needle.str[needle.len - delim_i - 1]
                  == haystack.str[haystack.len - i - 1]) {
        delim_i = (delim_i + 1) % needle.len;
        ++i;
    }
    /* Ugly logic to account for the reverse nature of this modulo search.
       the position needs to account for any part of the delim that may
       have started to match but then mismatched. The 1 is because
       this in an index being returned not a length. */
    return i == haystack.len ? haystack.len : haystack.len - i + delim_i - 1;
}

static inline size_t
min(size_t const a, size_t const b) {
    return a < b ? a : b;
}

static inline ptrdiff_t
signed_max(ptrdiff_t const a, ptrdiff_t const b) {
    return a > b ? a : b;
}

static inline SV_Order
char_compare(char const a, char const b) {
    return (a > b) - (a < b);
}

/* ======================   Static Utilities    =========================== */

/* This is section is modeled after the musl string.h library. However,
   using SV_Str_view that may not be null terminated requires modifications. */

#define BITOP(a, b, op)                                                        \
    ((a)[(size_t)(b) / (8 * sizeof *(a))] op(size_t) 1                         \
     << ((size_t)(b) % (8 * sizeof *(a))))

/* This is dangerous. Do not use this under normal circumstances.
   This is an internal helper for the backwards two way string
   searching algorithm. It expects that both arguments are
   greater than or equal to n bytes in length similar to how
   the forward version expects the same. However, the comparison
   moves backward from the location provided for n bytes. */
static int
reverse_memcmp(void const *const vl, void const *const vr, size_t n) {
    unsigned char const *l = vl;
    unsigned char const *r = vr;
    for (; n && *l == *r; n--, l--, r--) {}
    return n ? *l - *r : 0;
}

/* strcspn is based on musl C-standard library implementation
   http://git.musl-libc.org/cgit/musl/tree/src/string/strcspn.c
   A custom implementation is necessary because C standard library impls
   have no concept of a string view and will continue searching beyond the
   end of a view until null is found. This way, string searches are
   efficient and only within the range specified. */
static size_t
view_complimentary_substring_length(size_t const str_size,
                                    char const ARR_CONST_GEQ(str, str_size),
                                    size_t const set_size,
                                    char const ARR_GEQ(set, set_size)) {
    if (!set_size) {
        return str_size;
    }
    char const *a = str;
    size_t byteset[32 / sizeof(size_t)];
    if (set_size == 1) {
        for (size_t i = 0; i < str_size && *a != *set; ++a, ++i) {}
        return a - str;
    }
    memset(byteset, 0, sizeof byteset);
    for (size_t i = 0;
         i < set_size && BITOP(byteset, *(unsigned char *)set, |=);
         ++set, ++i) {}
    for (size_t i = 0; i < str_size && !BITOP(byteset, *(unsigned char *)a, &);
         ++a) {}
    return a - str;
}

/* strspn is based on musl C-standard library implementation
   https://git.musl-libc.org/cgit/musl/tree/src/string/strspn.c
   A custom implemenatation is necessary because C standard library impls
   have no concept of a string view and will continue searching beyond the
   end of a view until null is found. This way, string searches are
   efficient and only within the range specified. */
static size_t
view_substring_length(size_t const str_size,
                      char const ARR_CONST_GEQ(str, str_size),
                      size_t const set_size,
                      char const ARR_GEQ(set, set_size)) {
    char const *a = str;
    size_t byteset[32 / sizeof(size_t)] = {0};
    if (!set_size) {
        return str_size;
    }
    if (set_size == 1) {
        for (size_t i = 0; i < str_size && *a == *set; ++a, ++i) {}
        return a - str;
    }
    for (size_t i = 0;
         i < set_size && BITOP(byteset, *(unsigned char *)set, |=);
         ++set, ++i) {}
    for (size_t i = 0; i < str_size && BITOP(byteset, *(unsigned char *)a, &);
         ++a, ++i) {}
    return a - str;
}

/* Providing strnstrn rather than strstr at the lowest level works better
   for string views where the string may not be null terminated. There needs
   to always be the additional constraint that a search cannot exceed the
   haystack length. Returns 0 based index position at which needle begins in
   haystack if it can be found, otherwise the haystack size is returned. */
static size_t
view_match(ptrdiff_t const haystack_size,
           char const ARR_CONST_GEQ(haystack, haystack_size),
           ptrdiff_t const needle_size,
           char const ARR_CONST_GEQ(needle, needle_size)) {
    if (!haystack_size || !needle_size || needle_size > haystack_size) {
        return haystack_size;
    }
    if (1 == needle_size) {
        return view_match_char(haystack_size, haystack, *needle);
    }
    if (2 == needle_size) {
        return two_byte_view_match(haystack_size, (unsigned char *)haystack, 2,
                                   (unsigned char *)needle);
    }
    if (3 == needle_size) {
        return three_byte_view_match(haystack_size, (unsigned char *)haystack,
                                     3, (unsigned char *)needle);
    }
    if (4 == needle_size) {
        return four_byte_view_match(haystack_size, (unsigned char *)haystack, 4,
                                    (unsigned char *)needle);
    }
    return two_way_match(haystack_size, haystack, needle_size, needle);
}

/* For now reverse logic for backwards searches has been separated into
   other functions. There is a possible formula to unit the reverse and
   forward logic into one set of functions, but the code is ugly. See
   the start of the reverse two-way algorithm for more. May unite if
   a clean way exists. */
static size_t
reverse_view_match(ptrdiff_t const haystack_size,
                   char const ARR_CONST_GEQ(haystack, haystack_size),
                   ptrdiff_t const needle_size,
                   char const ARR_CONST_GEQ(needle, needle_size)) {
    if (!haystack_size || !needle_size || needle_size > haystack_size) {
        return haystack_size;
    }
    if (1 == needle_size) {
        return reverse_view_match_char(haystack_size, haystack, *needle);
    }
    if (2 == needle_size) {
        return reverse_two_byte_view_match(haystack_size,
                                           (unsigned char *)haystack, 2,
                                           (unsigned char *)needle);
    }
    if (3 == needle_size) {
        return reverse_three_byte_view_match(haystack_size,
                                             (unsigned char *)haystack, 3,
                                             (unsigned char *)needle);
    }
    if (4 == needle_size) {
        return reverse_four_byte_view_match(haystack_size,
                                            (unsigned char *)haystack, 4,
                                            (unsigned char *)needle);
    }
    return two_way_reverse_match(haystack_size, haystack, needle_size, needle);
}

/*==============   Post-Precomputation Two-Way Search    =================*/

/* Definitions for Two-Way String-Matching taken from original authors:

   CROCHEMORE M., PERRIN D., 1991, Two-way string-matching,
   Journal of the ACM 38(3):651-675.

   This algorithm is used for its simplicity and low space requirement.
   What follows is a massively simplified approximation (due to my own
   lack of knowledge) of glibc's approach with the help of the ESMAJ
   website on string matching and the original paper in ACM.

   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260

   Variable names and descriptions attempt to communicate authors' original
   meaning from the 1991 paper. */

/* Two Way string matching algorithm adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260

   Assumes the needle size is shorter than haystack size. Sizes are needed
   for string view operations because strings may not be null terminated
   and the string view library is most likely search a view rather than
   an entire string. Returns the position at which needle begins if found
   and the size of the haystack stack if not found. */
static inline size_t
two_way_match(ptrdiff_t const haystack_size,
              char const ARR_CONST_GEQ(haystack, haystack_size),
              ptrdiff_t const needle_size,
              char const ARR_CONST_GEQ(needle, needle_size)) {
    /* Preprocessing to get critical position and period distance. */
    struct Factorization const s = maximal_suffix(needle_size, needle);
    struct Factorization const r = maximal_suffix_reverse(needle_size, needle);
    struct Factorization const w
        = (s.critical_position > r.critical_position) ? s : r;
    /* Determine if memoization is available due to found border/overlap. */
    if (!memcmp(needle, needle + w.period_distance, w.critical_position + 1)) {
        return position_memoized(haystack_size, haystack, needle_size, needle,
                                 w.period_distance, w.critical_position);
    }
    return position_normal(haystack_size, haystack, needle_size, needle,
                           w.period_distance, w.critical_position);
}

/* Two Way string matching algorithm adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static size_t
position_memoized(ptrdiff_t const haystack_size,
                  char const ARR_CONST_GEQ(haystack, haystack_size),
                  ptrdiff_t const needle_size,
                  char const ARR_CONST_GEQ(needle, needle_size),
                  ptrdiff_t const period_dist, ptrdiff_t const critical_pos) {
    ptrdiff_t lpos = 0;
    ptrdiff_t rpos = 0;
    /* Eliminate worst case quadratic time complexity with memoization. */
    ptrdiff_t memoize_shift = -1;
    while (lpos <= haystack_size - needle_size) {
        rpos = signed_max(critical_pos, memoize_shift) + 1;
        while (rpos < needle_size && needle[rpos] == haystack[rpos + lpos]) {
            ++rpos;
        }
        if (rpos < needle_size) {
            lpos += (rpos - critical_pos);
            memoize_shift = -1;
            continue;
        }
        /* r_pos >= needle_size */
        rpos = critical_pos;
        while (rpos > memoize_shift && needle[rpos] == haystack[rpos + lpos]) {
            --rpos;
        }
        if (rpos <= memoize_shift) {
            return lpos;
        }
        lpos += period_dist;
        /* Some prefix of needle coincides with the text. Memoize the length
           of this prefix to increase length of next shift, if possible. */
        memoize_shift = needle_size - period_dist - 1;
    }
    return haystack_size;
}

/* Two Way string matching algorithm adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static size_t
position_normal(ptrdiff_t const haystack_size,
                char const ARR_CONST_GEQ(haystack, haystack_size),
                ptrdiff_t const needle_size,
                char const ARR_CONST_GEQ(needle, needle_size),
                ptrdiff_t period_dist, ptrdiff_t const critical_pos) {
    period_dist
        = signed_max(critical_pos + 1, needle_size - critical_pos - 1) + 1;
    ptrdiff_t lpos = 0;
    ptrdiff_t rpos = 0;
    while (lpos <= haystack_size - needle_size) {
        rpos = critical_pos + 1;
        while (rpos < needle_size && needle[rpos] == haystack[rpos + lpos]) {
            ++rpos;
        }
        if (rpos < needle_size) {
            lpos += (rpos - critical_pos);
            continue;
        }
        /* r_pos >= needle_size */
        rpos = critical_pos;
        while (rpos >= 0 && needle[rpos] == haystack[rpos + lpos]) {
            --rpos;
        }
        if (rpos < 0) {
            return lpos;
        }
        lpos += period_dist;
    }
    return haystack_size;
}

/* ==============   Suffix and Critical Factorization    =================*/

/* Computing of the maximal suffix. Adapted from ESMAJ.
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static inline struct Factorization
maximal_suffix(ptrdiff_t const needle_size,
               char const ARR_CONST_GEQ(needle, needle_size)) {
    ptrdiff_t suff_pos = -1;
    ptrdiff_t period = 1;
    ptrdiff_t last_rest = 0;
    ptrdiff_t rest = 1;
    while (last_rest + rest < needle_size) {
        switch (
            char_compare(needle[last_rest + rest], needle[suff_pos + rest])) {
            case SV_ORDER_LESSER:
                last_rest += rest;
                rest = 1;
                period = last_rest - suff_pos;
                break;
            case SV_ORDER_EQUAL:
                if (rest != period) {
                    ++rest;
                } else {
                    last_rest += period;
                    rest = 1;
                }
                break;
            case SV_ORDER_GREATER:
                suff_pos = last_rest;
                last_rest = suff_pos + 1;
                rest = period = 1;
                break;
            default:
                break;
        }
    }
    return (struct Factorization){
        .critical_position = suff_pos,
        .period_distance = period,
    };
}

/* Computing of the maximal suffix reverse. Sometimes called tilde.
   adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static inline struct Factorization
maximal_suffix_reverse(ptrdiff_t const needle_size,
                       char const ARR_CONST_GEQ(needle, needle_size)) {
    ptrdiff_t suff_pos = -1;
    ptrdiff_t period = 1;
    ptrdiff_t last_rest = 0;
    ptrdiff_t rest = 1;
    while (last_rest + rest < needle_size) {
        switch (
            char_compare(needle[last_rest + rest], needle[suff_pos + rest])) {
            case SV_ORDER_GREATER:
                last_rest += rest;
                rest = 1;
                period = last_rest - suff_pos;
                break;
            case SV_ORDER_EQUAL:
                if (rest != period) {
                    ++rest;
                } else {
                    last_rest += period;
                    rest = 1;
                }
                break;
            case SV_ORDER_LESSER:
                suff_pos = last_rest;
                last_rest = suff_pos + 1;
                rest = period = 1;
                break;
            default:
                break;
        }
    }
    return (struct Factorization){
        .critical_position = suff_pos,
        .period_distance = period,
    };
}

/*=======================  Right to Left Search  ===========================*/

/* Two way algorithm is easy to reverse. Instead of trying to reverse all
   logic in the factorizations and two way searches, leave the algorithms
   and calculate the values returned as offsets from the end of the string
   instead of indices starting from 0. It would be even be possible to unite
   these functions into one with the following formula
   (using the suffix calculation as an example):

        ptrdiff_t suff_pos = -1;
        ptrdiff_t period = 1;
        ptrdiff_t last_rest = 0;
        ptrdiff_t rest = 1;
        ptrdiff_t negate_size = 0;
        ptrdiff_t negate_one = 0;
        if (direction == FORWARD)
        {
            negate_size = needle_size;
            negate_one = 1;
        }
        while (last_rest + rest < needle_size)
        {
            switch (SV_char_cmp(
                needle[needle_size
                        - (last_rest + rest) - 1 + negate_size + negate_one],
                needle[needle_size
                        - (suff_pos + rest) - 1 + negate_size + negate_one]))
            {
            ...

   That would save the code repitition across all of the following
   functions but probably would make the code even harder to read and
   maintain. These algorithms are dense enough already so I think repetion
   is fine. Leaving comment here if that changes or an even better way comes
   along. */

/* Searches a string from right to left with a two-way algorithm. Returns
   the position of the start of the strig if found and string size if not. */
static inline size_t
two_way_reverse_match(ptrdiff_t const haystack_size,
                      char const ARR_CONST_GEQ(haystack, haystack_size),
                      ptrdiff_t const needle_size,
                      char const ARR_CONST_GEQ(needle, needle_size)) {
    struct Factorization const s = reverse_maximal_suffix(needle_size, needle);
    struct Factorization const r
        = reverse_maximal_suffix_reverse(needle_size, needle);
    struct Factorization const w
        = (s.critical_position > r.critical_position) ? s : r;
    if (!reverse_memcmp(needle + needle_size - 1,
                        needle + needle_size - w.period_distance - 1,
                        w.critical_position + 1)) {
        return reverse_position_memoized(haystack_size, haystack, needle_size,
                                         needle, w.period_distance,
                                         w.critical_position);
    }
    return reverse_position_normal(haystack_size, haystack, needle_size, needle,
                                   w.period_distance, w.critical_position);
}

static size_t
reverse_position_memoized(ptrdiff_t const haystack_size,
                          char const ARR_CONST_GEQ(haystack, haystack_size),
                          ptrdiff_t const needle_size,
                          char const ARR_CONST_GEQ(needle, needle_size),
                          ptrdiff_t const period_dist,
                          ptrdiff_t const critical_pos) {
    ptrdiff_t lpos = 0;
    ptrdiff_t rpos = 0;
    ptrdiff_t memoize_shift = -1;
    while (lpos <= haystack_size - needle_size) {
        rpos = signed_max(critical_pos, memoize_shift) + 1;
        while (rpos < needle_size
               && needle[needle_size - rpos - 1]
                      == haystack[haystack_size - (rpos + lpos) - 1]) {
            ++rpos;
        }
        if (rpos < needle_size) {
            lpos += (rpos - critical_pos);
            memoize_shift = -1;
            continue;
        }
        /* r_pos >= needle_size */
        rpos = critical_pos;
        while (rpos > memoize_shift
               && needle[needle_size - rpos - 1]
                      == haystack[haystack_size - (rpos + lpos) - 1]) {
            --rpos;
        }
        if (rpos <= memoize_shift) {
            return haystack_size - lpos - needle_size;
        }
        lpos += period_dist;
        /* Some prefix of needle coincides with the text. Memoize the length
           of this prefix to increase length of next shift, if possible. */
        memoize_shift = needle_size - period_dist - 1;
    }
    return haystack_size;
}

static size_t
reverse_position_normal(ptrdiff_t const haystack_size,
                        char const ARR_CONST_GEQ(haystack, haystack_size),
                        ptrdiff_t const needle_size,
                        char const ARR_CONST_GEQ(needle, needle_size),
                        ptrdiff_t period_dist, ptrdiff_t const critical_pos) {
    period_dist
        = signed_max(critical_pos + 1, needle_size - critical_pos - 1) + 1;
    ptrdiff_t lpos = 0;
    ptrdiff_t rpos = 0;
    while (lpos <= haystack_size - needle_size) {
        rpos = critical_pos + 1;
        while (rpos < needle_size
               && (needle[needle_size - rpos - 1]
                   == haystack[haystack_size - (rpos + lpos) - 1])) {
            ++rpos;
        }
        if (rpos < needle_size) {
            lpos += (rpos - critical_pos);
            continue;
        }
        /* r_pos >= needle_size */
        rpos = critical_pos;
        while (rpos >= 0
               && needle[needle_size - rpos - 1]
                      == haystack[haystack_size - (rpos + lpos) - 1]) {
            --rpos;
        }
        if (rpos < 0) {
            return haystack_size - lpos - needle_size;
        }
        lpos += period_dist;
    }
    return haystack_size;
}

static inline struct Factorization
reverse_maximal_suffix(ptrdiff_t const needle_size,
                       char const ARR_CONST_GEQ(needle, needle_size)) {
    ptrdiff_t suff_pos = -1;
    ptrdiff_t period = 1;
    ptrdiff_t last_rest = 0;
    ptrdiff_t rest = 1;
    while (last_rest + rest < needle_size) {
        switch (char_compare(needle[needle_size - (last_rest + rest) - 1],
                             needle[needle_size - (suff_pos + rest) - 1])) {
            case SV_ORDER_LESSER:
                last_rest += rest;
                rest = 1;
                period = last_rest - suff_pos;
                break;
            case SV_ORDER_EQUAL:
                if (rest != period) {
                    ++rest;
                } else {
                    last_rest += period;
                    rest = 1;
                }
                break;
            case SV_ORDER_GREATER:
                suff_pos = last_rest;
                last_rest = suff_pos + 1;
                rest = period = 1;
                break;
            default:
                break;
        }
    }
    return (struct Factorization){
        .critical_position = suff_pos,
        .period_distance = period,
    };
}

static inline struct Factorization
reverse_maximal_suffix_reverse(ptrdiff_t const needle_size,
                               char const ARR_CONST_GEQ(needle, needle_size)) {
    ptrdiff_t suff_pos = -1;
    ptrdiff_t period = 1;
    ptrdiff_t last_rest = 0;
    ptrdiff_t rest = 1;
    while (last_rest + rest < needle_size) {
        switch (char_compare(needle[needle_size - (last_rest + rest) - 1],
                             needle[needle_size - (suff_pos + rest) - 1])) {
            case SV_ORDER_GREATER:
                last_rest += rest;
                rest = 1;
                period = last_rest - suff_pos;
                break;
            case SV_ORDER_EQUAL:
                if (rest != period) {
                    ++rest;
                } else {
                    last_rest += period;
                    rest = 1;
                }
                break;
            case SV_ORDER_LESSER:
                suff_pos = last_rest;
                last_rest = suff_pos + 1;
                rest = period = 1;
                break;
            default:
                break;
        }
    }
    return (struct Factorization){
        .critical_position = suff_pos,
        .period_distance = period,
    };
}

/* ======================   Brute Force Search    ==========================
 */

/* All brute force searches adapted from musl C library.
   http://git.musl-libc.org/cgit/musl/tree/src/string/strstr.c
   They must stop the search at haystack size and therefore required slight
   modification because string views may not be null terminated. Reverse
   methods are my own additions provided to support a compliant reverse
   search for rfind which most string libraries specify must search right
   to left. Also having a reverse tokenizer is convenient and also relies
   on right to left brute force searches. */
static inline size_t
view_match_char(size_t n, char const ARR_GEQ(s, n), char const c) {
    size_t i = 0;
    for (; n && *s != c; s++, --n, ++i) {}
    return i;
}

static inline size_t
reverse_view_match_char(size_t const n, char const ARR_CONST_GEQ(s, n),
                        char const c) {
    char const *x = s + n - 1;
    size_t i = n;
    for (; i && *x != c; x--, --i) {}
    return i ? i - 1 : n;
}

static inline size_t
two_byte_view_match(size_t const size, unsigned char const ARR_GEQ(h, size),
                    size_t const n_size,
                    unsigned char const ARR_CONST_GEQ(n, n_size)) {
    unsigned char const *const end = h + size;
    uint16_t nw = n[0] << 8 | n[1];
    uint16_t hw = h[0] << 8 | h[1];
    for (++h; hw != nw && ++h < end; hw = (hw << 8) | *h) {}
    return h >= end ? size : (size - (size_t)(end - h)) - 1;
}

static inline size_t
reverse_two_byte_view_match(size_t const size,
                            unsigned char const ARR_CONST_GEQ(h, size),
                            size_t const n_size,
                            unsigned char const ARR_CONST_GEQ(n, n_size)) {
    unsigned char const *i = h + (size - 2);
    uint16_t nw = n[0] << 8 | n[1];
    uint16_t iw = i[0] << 8 | i[1];
    /* The search is right to left therefore the Most Significant Byte will
       be the leading character of the string and the previous leading
       character is shifted to the right. */
    for (; iw != nw && --i >= h; iw = (iw >> 8) | (*i << 8)) {}
    return i < h ? size : (size_t)(i - h);
}

static inline size_t
three_byte_view_match(size_t const size, unsigned char const ARR_GEQ(h, size),
                      size_t const n_size,
                      unsigned char const ARR_CONST_GEQ(n, n_size)) {
    unsigned char const *const end = h + size;
    uint32_t nw = (uint32_t)n[0] << 24 | n[1] << 16 | n[2] << 8;
    uint32_t hw = (uint32_t)h[0] << 24 | h[1] << 16 | h[2] << 8;
    for (h += 2; hw != nw && ++h < end; hw = (hw | *h) << 8) {}
    return h >= end ? size : (size - (size_t)(end - h)) - 2;
}

static inline size_t
reverse_three_byte_view_match(size_t const size,
                              unsigned char const ARR_CONST_GEQ(h, size),
                              size_t const n_size,
                              unsigned char const ARR_CONST_GEQ(n, n_size)) {
    unsigned char const *i = h + (size - 3);
    uint32_t nw = (uint32_t)n[0] << 16 | n[1] << 8 | n[2];
    uint32_t iw = (uint32_t)i[0] << 16 | i[1] << 8 | i[2];
    /* Align the bits with fewer left shifts such that as the parsing
       progresses right left, the leading character always takes highest
       bit position and there is no need for any masking. */
    for (; iw != nw && --i >= h; iw = (iw >> 8) | (*i << 16)) {}
    return i < h ? size : (size_t)(i - h);
}

static inline size_t
four_byte_view_match(size_t const size, unsigned char const ARR_GEQ(h, size),
                     size_t const n_size,
                     unsigned char const ARR_CONST_GEQ(n, n_size)) {
    unsigned char const *const end = h + size;
    uint32_t nw = (uint32_t)n[0] << 24 | n[1] << 16 | n[2] << 8 | n[3];
    uint32_t hw = (uint32_t)h[0] << 24 | h[1] << 16 | h[2] << 8 | h[3];
    for (h += 3; hw != nw && ++h < end; hw = (hw << 8) | *h) {}
    return h >= end ? size : (size - (size_t)(end - h)) - 3;
}

static inline size_t
reverse_four_byte_view_match(size_t const size,
                             unsigned char const ARR_CONST_GEQ(h, size),
                             size_t const n_size,
                             unsigned char const ARR_CONST_GEQ(n, n_size)) {
    unsigned char const *i = h + (size - 4);
    uint32_t nw = (uint32_t)n[0] << 24 | n[1] << 16 | n[2] << 8 | n[3];
    uint32_t iw = (uint32_t)i[0] << 24 | i[1] << 16 | i[2] << 8 | i[3];
    /* Now that all four bytes of the unsigned int are used the shifting
       becomes more intuitive. The window slides left to right and the
       next leading character takes the high bit position. */
    for (; iw != nw && --i >= h; iw = (iw >> 8) | (*i << 24)) {}
    return i < h ? size : (size_t)(i - h);
}
