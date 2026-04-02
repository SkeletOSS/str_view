#ifndef TEST
#define TEST

#include <stdio.h> /* IWYU pragma: keep */

#define RED "\033[38;5;9m"
#define GREEN "\033[38;5;10m"
#define CYAN "\033[38;5;14m"
#define NONE "\033[0m"

enum Test_result {
    ERROR = -1,
    PASS,
    FAIL,
};

typedef enum Test_result (*Test_fn)(void);

/* The CHECK macro evaluates a result against an expecation as one
   may be familiar with in many testing frameworks. However, it is
   expected to execute in a function where a test_result is returned.
   Provide the resulting operation against the expected outcome. The
   types must be comparable with ==/!=. Finally, provide the type and
   format specifier for the types being compared which also must be
   the same for both RESULT and EXPECTED (e.g. int, "%d", size_t, "%zu",
   bool, "%d"). If RESULT or EXPECTED are function calls they will only
   be evaluated once, as expected. */
#define CHECK(RESULT, EXPECTED, TYPE, TYPE_FORMAT_SPECIFIER)                   \
    do {                                                                       \
        const TYPE _result = RESULT;                                           \
        const TYPE _expected = EXPECTED;                                       \
        if (_result != _expected) {                                            \
            (void)fprintf(stderr, CYAN "--\nfailure in %s, line %d\n" NONE,    \
                          __func__, __LINE__);                                 \
            (void)fprintf(stderr,                                              \
                          GREEN "CHECK: "                                      \
                                "RESULT( %s ) == EXPECTED( %s )" NONE "\n",    \
                          #RESULT, #EXPECTED);                                 \
            (void)fprintf(stderr, RED "ERROR: RESULT( ");                      \
            (void)fprintf(stderr, TYPE_FORMAT_SPECIFIER, _result);             \
            (void)fprintf(stderr, " ) != EXPECTED( ");                         \
            (void)fprintf(stderr, TYPE_FORMAT_SPECIFIER, _expected);           \
            (void)fprintf(stderr, " )" CYAN "\n" NONE);                        \
            return FAIL;                                                       \
        }                                                                      \
    } while (0)

#endif
