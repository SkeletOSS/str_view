#include "str_view.h"
#include "test.h"

#include <stdbool.h>
#include <string.h>

static enum Test_result test_from_null(void);
static enum Test_result test_from_delim(void);
static enum Test_result test_from_delim_multiple(void);
static enum Test_result test_from_multichar_delim(void);
static enum Test_result test_from_delim_no_delim(void);
static enum Test_result test_empty_constructor(void);

#define NUM_TESTS (size_t)6
static Test_fn const all_tests[NUM_TESTS] = {
    test_from_null,           test_from_delim,
    test_from_delim_multiple, test_from_multichar_delim,
    test_from_delim_no_delim, test_empty_constructor,
};

int
main(void) {
    enum Test_result res = PASS;
    for (size_t i = 0; i < NUM_TESTS; ++i) {
        enum Test_result const t_res = all_tests[i]();
        if (t_res == FAIL) {
            res = FAIL;
        }
    }
    return res;
}

static enum Test_result
test_from_null(void) {
    char const *const reference = "Don't miss the terminator!";
    SV_Str_view const s = SV_from_terminated(reference);
    size_t const reference_len = strlen(reference);
    CHECK(reference_len, SV_len(s), size_t, "%zu");
    CHECK(SV_terminated_compare(s, reference), SV_ORDER_EQUAL, SV_Order, "%d");
    char const *const chunk = "Don't";
    size_t const chunk_len = strlen(chunk);
    SV_Str_view const n_bytes = SV_from_view(chunk_len, reference);
    CHECK(SV_len(n_bytes), chunk_len, size_t, "%zu");
    CHECK(SV_terminated_compare(n_bytes, chunk), SV_ORDER_EQUAL, SV_Order,
          "%d");
    return PASS;
}

static enum Test_result
test_from_delim(void) {
    char const *const reference = "Don'tmissthedelim That was it!";
    char const *const reference_chunk = "Don'tmissthedelim";
    SV_Str_view const sv = SV_from_delimiter(reference, " ");
    size_t const reference_len = strlen(reference_chunk);
    CHECK(reference_len, SV_len(sv), size_t, "%zu");
    CHECK(SV_terminated_compare(sv, reference_chunk), SV_ORDER_EQUAL, SV_Order,
          "%d");
    /* If the string starts with delim we must skip it. */
    char const *const ref2 = ",Don't miss the delim, that was it!";
    char const *const ref2_chunk = "Don't miss the delim";
    SV_Str_view const sv2 = SV_from_delimiter(ref2, ",");
    size_t const ref2_len = strlen(ref2_chunk);
    CHECK(ref2_len, SV_len(sv2), size_t, "%zu");
    CHECK(SV_terminated_compare(sv2, ref2_chunk), SV_ORDER_EQUAL, SV_Order,
          "%d");
    CHECK(SV_terminated_compare(SV_extend(sv2), ref2 + 1), SV_ORDER_EQUAL,
          SV_Order, "%d");
    return PASS;
}

static enum Test_result
test_from_delim_multiple(void) {
    char const *const reference = ",,,Don'tmissthedelim,,,That was it!";
    char const *const reference_chunk = "Don'tmissthedelim";
    SV_Str_view const sv = SV_from_delimiter(reference, ",");
    size_t const reference_len = strlen(reference_chunk);
    CHECK(reference_len, SV_len(sv), size_t, "%zu");
    CHECK(SV_terminated_compare(sv, reference_chunk), SV_ORDER_EQUAL, SV_Order,
          "%d");
    return PASS;
}

static enum Test_result
test_from_multichar_delim(void) {
    char const *const reference = "delimDon'tmissthedelimThat was it!";
    char const *const reference_chunk = "Don'tmissthe";
    SV_Str_view const sv = SV_from_delimiter(reference, "delim");
    size_t const reference_len = strlen(reference_chunk);
    CHECK(reference_len, SV_len(sv), size_t, "%zu");
    CHECK(SV_terminated_compare(sv, reference_chunk), SV_ORDER_EQUAL, SV_Order,
          "%d");
    return PASS;
}

static enum Test_result
test_from_delim_no_delim(void) {
    char const *const reference = "Don'tmissthedelimbutnodelim!";
    SV_Str_view const sv = SV_from_delimiter(reference, " ");
    size_t const reference_len = strlen(reference);
    CHECK(reference_len, SV_len(sv), size_t, "%zu");
    CHECK(reference[reference_len - 1], SV_at(sv, SV_len(sv) - 1), char, "%c");
    return PASS;
}

static enum Test_result
test_empty_constructor(void) {
    char const *const reference = "------------";
    SV_Str_view const sv = SV_from_delimiter(reference, "-");
    CHECK(SV_len(sv), 0UL, size_t, "%zu");
    CHECK(SV_is_empty(sv), true, bool, "%d");
    return PASS;
}
