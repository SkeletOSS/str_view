#include "str_view.h"
#include "test.h"

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static enum Test_result test_small_find(void);
static enum Test_result test_small_rfind(void);
static enum Test_result test_rfind_off_by_one(void);
static enum Test_result test_find_of_sets(void);
static enum Test_result test_substring_brute_force(void);
static enum Test_result test_rfind_brute_force(void);
static enum Test_result test_find_rfind_memoization(void);
static enum Test_result test_consecutive_find(void);
static enum Test_result test_consecutive_rfind(void);
static enum Test_result test_substring_off_by_one(void);
static enum Test_result test_substring_search(void);
static enum Test_result test_rsubstring_search(void);
static enum Test_result test_long_substring(void);

#define NUM_TESTS (size_t)13

static Test_fn const all_tests[NUM_TESTS] = {
    test_small_find,
    test_small_rfind,
    test_find_of_sets,
    test_substring_brute_force,
    test_rfind_brute_force,
    test_rfind_off_by_one,
    test_find_rfind_memoization,
    test_consecutive_find,
    test_consecutive_rfind,
    test_substring_off_by_one,
    test_substring_search,
    test_rsubstring_search,
    test_long_substring,
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
test_small_find(void) {
    char const ref[20] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '!',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '*',
        [10] = ' ', [11] = '_', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    SV_Str_view str = SV_from_terminated(ref);
    CHECK(SV_find(str, 0, SV_from_terminated("C")), 2UL, size_t, "%zu");
    CHECK(SV_find(str, 0, SV_from_terminated("")), 19UL, size_t, "%zu");
    CHECK(SV_find(str, 0, SV_from_terminated("_")), 11UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_small_rfind(void) {
    char const ref[20] = {
        [0] = 'Y',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '!',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '*',
        [10] = ' ', [11] = '_', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    SV_Str_view str = SV_from_terminated(ref);
    CHECK(SV_reverse_find(str, SV_len(str), SV_from_terminated("!")), 16UL,
          size_t, "%zu");
    CHECK(SV_reverse_find(str, SV_len(str), SV_from_terminated("Y")), 0UL,
          size_t, "%zu");
    CHECK(SV_reverse_find(str, SV_len(str), SV_from_terminated("X")), 19UL,
          size_t, "%zu");
    return PASS;
}

static enum Test_result
test_find_of_sets(void) {
    char const ref[25] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = 'B',  [4] = '!',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '.',
        [10] = ':', [11] = ';', [12] = ',', [13] = ' ', [14] = '?',
        [15] = ' ', [16] = '_', [17] = '_', [18] = ' ', [19] = '!',
        [20] = '!', [21] = '!', [22] = 'Z', [23] = 'z', [24] = '\0',
    };
    SV_Str_view str = SV_from_terminated(ref);
    CHECK(SV_find_first_of(str, SV_from_terminated("CB!")), 2UL, size_t, "%zu");
    CHECK(SV_find_first_of(str, SV_from_terminated("")), 24UL, size_t, "%zu");
    CHECK(SV_find_last_of(str, SV_from_terminated("! _")), 21UL, size_t, "%zu");
    CHECK(SV_find_last_not_of(str, SV_from_terminated("CBA!")), 22UL, size_t,
          "%zu");
    CHECK(SV_find_first_not_of(str, SV_from_terminated("ACB!;:, *.")), 14UL,
          size_t, "%zu");
    return PASS;
}

static enum Test_result
test_substring_brute_force(void) {
    char const *one_byte_needle = "A";
    char const *two_byte_needle = "AA";
    char const *three_byte_needle = "AAA";
    char const *four_byte_needle = "AAAA";
    char const *needle = "find the needle!";
    char const *const haystack = "__A__AA___AAA___AAAA___find the needle!___";
    SV_Str_view const haystack_view = SV_from_terminated(haystack);

    SV_Str_view const one_byte_view
        = SV_match(haystack_view, SV_from_terminated(one_byte_needle));
    CHECK(SV_terminated_compare(one_byte_view, one_byte_needle), SV_ORDER_EQUAL,
          SV_Order, "%d");
    SV_Str_view const two_byte_view
        = SV_match(haystack_view, SV_from_terminated(two_byte_needle));
    CHECK(SV_terminated_compare(two_byte_view, two_byte_needle), SV_ORDER_EQUAL,
          SV_Order, "%d");
    SV_Str_view const three_byte_view
        = SV_match(haystack_view, SV_from_terminated(three_byte_needle));
    CHECK(SV_terminated_compare(three_byte_view, three_byte_needle),
          SV_ORDER_EQUAL, SV_Order, "%d");
    SV_Str_view const four_byte_view
        = SV_match(haystack_view, SV_from_terminated(four_byte_needle));
    CHECK(SV_terminated_compare(four_byte_view, four_byte_needle),
          SV_ORDER_EQUAL, SV_Order, "%d");
    SV_Str_view const needle_view
        = SV_match(haystack_view, SV_from_terminated(needle));
    CHECK(SV_terminated_compare(needle_view, needle), SV_ORDER_EQUAL, SV_Order,
          "%d");
    char const *one_byte_found = strstr(haystack, one_byte_needle);
    CHECK(one_byte_found, SV_begin(one_byte_view), char *const, "%s");
    char const *two_byte_found = strstr(haystack, two_byte_needle);
    CHECK(two_byte_found, SV_begin(two_byte_view), char *const, "%s");
    char const *three_byte_found = strstr(haystack, three_byte_needle);
    CHECK(three_byte_found, SV_begin(three_byte_view), char *const, "%s");
    char const *four_byte_found = strstr(haystack, four_byte_needle);
    CHECK(four_byte_found, SV_begin(four_byte_view), char *const, "%s");
    char const *needle_found = strstr(haystack, needle);
    CHECK(needle_found, SV_begin(needle_view), char *const, "%s");
    SV_Str_view const one_byte_fail
        = SV_match(haystack_view, SV_from_terminated("J"));
    CHECK(SV_len(one_byte_fail), 0UL, size_t, "%zu");
    SV_Str_view const two_byte_fail
        = SV_match(haystack_view, SV_from_terminated("XY"));
    CHECK(SV_len(two_byte_fail), 0UL, size_t, "%zu");
    SV_Str_view const three_byte_fail
        = SV_match(haystack_view, SV_from_terminated("ZZY"));
    CHECK(SV_len(three_byte_fail), 0UL, size_t, "%zu");
    SV_Str_view const four_byte_fail
        = SV_match(haystack_view, SV_from_terminated("8888"));
    CHECK(SV_len(four_byte_fail), 0UL, size_t, "%zu");
    SV_Str_view const needle_fail
        = SV_match(haystack_view, SV_from_terminated("this is failure"));
    CHECK(SV_len(needle_fail), 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_rfind_brute_force(void) {
    char const *one_byte_needle = "A";
    char const *two_byte_needle = "BB";
    char const *three_byte_needle = "CCC";
    char const *four_byte_needle = "DDDD";
    char const *needle = "find the needle!";
    char const *const haystack = "++DDDD++CCC+++BB+++A+++find the needle!+++";
    char const *one_byte_found = strstr(haystack, one_byte_needle);
    char const *two_byte_found = strstr(haystack, two_byte_needle);
    char const *three_byte_found = strstr(haystack, three_byte_needle);
    char const *four_byte_found = strstr(haystack, four_byte_needle);
    char const *needle_found = strstr(haystack, needle);
    SV_Str_view const haystack_view = SV_from_terminated(haystack);
    size_t const one_byte_pos
        = SV_reverse_find(haystack_view, SV_len(haystack_view),
                          SV_from_terminated(one_byte_needle));
    CHECK(one_byte_pos, (size_t)(one_byte_found - haystack), size_t, "%zu");
    size_t const two_byte_pos
        = SV_reverse_find(haystack_view, SV_len(haystack_view),
                          SV_from_terminated(two_byte_needle));
    CHECK(two_byte_pos, (size_t)(two_byte_found - haystack), size_t, "%zu");
    size_t const three_byte_pos
        = SV_reverse_find(haystack_view, SV_len(haystack_view),
                          SV_from_terminated(three_byte_needle));
    CHECK(three_byte_pos, (size_t)(three_byte_found - haystack), size_t, "%zu");
    size_t const four_byte_pos
        = SV_reverse_find(haystack_view, SV_len(haystack_view),
                          SV_from_terminated(four_byte_needle));
    CHECK(four_byte_pos, (size_t)(four_byte_found - haystack), size_t, "%zu");
    size_t const needle_pos = SV_reverse_find(
        haystack_view, SV_len(haystack_view), SV_from_terminated(needle));
    CHECK(needle_pos, (size_t)(needle_found - haystack), size_t, "%zu");

    SV_Str_view const one_byte_rsvsv
        = SV_reverse_match(haystack_view, SV_from_terminated(one_byte_needle));
    CHECK(SV_terminated_compare(one_byte_rsvsv, one_byte_needle),
          SV_ORDER_EQUAL, SV_Order, "%d");
    SV_Str_view const two_byte_rsvsv
        = SV_reverse_match(haystack_view, SV_from_terminated(two_byte_needle));
    CHECK(SV_terminated_compare(two_byte_rsvsv, two_byte_needle),
          SV_ORDER_EQUAL, SV_Order, "%d");
    SV_Str_view const three_byte_rsvsv = SV_reverse_match(
        haystack_view, SV_from_terminated(three_byte_needle));
    CHECK(SV_terminated_compare(three_byte_rsvsv, three_byte_needle),
          SV_ORDER_EQUAL, SV_Order, "%d");
    SV_Str_view const four_byte_rsvsv
        = SV_reverse_match(haystack_view, SV_from_terminated(four_byte_needle));
    CHECK(SV_terminated_compare(four_byte_rsvsv, four_byte_needle),
          SV_ORDER_EQUAL, SV_Order, "%d");
    SV_Str_view const needle_rsvsv
        = SV_reverse_match(haystack_view, SV_from_terminated(needle));
    CHECK(SV_terminated_compare(needle_rsvsv, needle), SV_ORDER_EQUAL, SV_Order,
          "%d");

    size_t const one_byte_fail = SV_reverse_find(
        haystack_view, SV_len(haystack_view), SV_from_terminated("J"));
    CHECK(one_byte_fail, SV_len(haystack_view), size_t, "%zu");
    size_t const two_byte_fail = SV_reverse_find(
        haystack_view, SV_len(haystack_view), SV_from_terminated("ZZ"));
    CHECK(two_byte_fail, SV_len(haystack_view), size_t, "%zu");
    size_t const three_byte_fail = SV_reverse_find(
        haystack_view, SV_len(haystack_view), SV_from_terminated("888"));
    CHECK(three_byte_fail, SV_len(haystack_view), size_t, "%zu");
    size_t const four_byte_fail = SV_reverse_find(
        haystack_view, SV_len(haystack_view), SV_from_terminated("1738"));
    CHECK(four_byte_fail, SV_len(haystack_view), size_t, "%zu");
    size_t const needle_fail
        = SV_reverse_find(haystack_view, SV_len(haystack_view),
                          SV_from_terminated("this is a failure"));
    CHECK(needle_fail, SV_len(haystack_view), size_t, "%zu");
    SV_Str_view const one_byte_fail_rsvsv
        = SV_reverse_match(haystack_view, SV_from_terminated("J"));
    CHECK(SV_is_empty(one_byte_fail_rsvsv), true, bool, "%d");
    SV_Str_view const two_byte_fail_rsvsv
        = SV_reverse_match(haystack_view, SV_from_terminated("ZZ"));
    CHECK(SV_is_empty(two_byte_fail_rsvsv), true, bool, "%d");
    SV_Str_view const three_byte_fail_rsvsv
        = SV_reverse_match(haystack_view, SV_from_terminated("888"));
    CHECK(SV_is_empty(three_byte_fail_rsvsv), true, bool, "%d");
    SV_Str_view const four_byte_fail_rsvsv
        = SV_reverse_match(haystack_view, SV_from_terminated("1738"));
    CHECK(SV_is_empty(four_byte_fail_rsvsv), true, bool, "%d");
    SV_Str_view const needle_fail_rsvsv = SV_reverse_match(
        haystack_view, SV_from_terminated("this is a failure"));
    CHECK(SV_is_empty(needle_fail_rsvsv), true, bool, "%d");
    return PASS;
}

static enum Test_result
test_consecutive_find(void) {
    char const needles[13] = {
        [0] = 'a',  [1] = 'a',  [2] = 'a',   [3] = 'Z', [4] = 'a',
        [5] = 'a',  [6] = 'Z',  [7] = 'a',   [8] = 'a', [9] = 'a',
        [10] = 'a', [11] = 'Z', [12] = '\0',
    };
    size_t const found_positions[3] = {3, 6, 11};
    size_t const size = sizeof(found_positions) / sizeof(found_positions[0]);
    SV_Str_view const hay = SV_from_terminated(needles);
    SV_Str_view const needle = SV_from_terminated("Z");
    size_t pos = 0;
    size_t i = 0;
    bool found = false;
    while (i < size && (pos = SV_find(hay, pos, needle)) != SV_npos(hay)) {
        found = true;
        CHECK(pos, found_positions[i], size_t, "%zu");
        ++pos;
        ++i;
    }
    CHECK(found, true, bool, "%d");
    CHECK(i, size, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_consecutive_rfind(void) {
    char const needles[13] = {
        [0] = 'a',  [1] = 'a',  [2] = 'a',   [3] = 'Z', [4] = 'a',
        [5] = 'a',  [6] = 'Z',  [7] = 'a',   [8] = 'a', [9] = 'a',
        [10] = 'a', [11] = 'Z', [12] = '\0',
    };
    size_t const found_positions[3] = {3, 6, 11};
    SV_Str_view const hay = SV_from_terminated(needles);
    SV_Str_view const needle = SV_from_terminated("Z");
    size_t pos = SV_len(hay);
    size_t i = sizeof(found_positions) / sizeof(found_positions[0]);
    bool found = false;
    while (i && (pos = SV_reverse_find(hay, pos, needle)) != SV_npos(hay)) {
        --i;
        found = true;
        CHECK(pos, found_positions[i], size_t, "%zu");
        --pos;
    }
    CHECK(found, true, bool, "%d");
    CHECK(i, 0ULL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_rfind_off_by_one(void) {
    char const *one_byte_needle = "Z";
    CHECK(SV_reverse_find(SV_from_terminated(one_byte_needle), 1,
                          SV_from_terminated("Z")),
          0UL, size_t, "%zu");
    CHECK(SV_reverse_find(SV_from_terminated(one_byte_needle), 1,
                          SV_from_terminated("A")),
          1UL, size_t, "%zu");
    char const *two_byte_needle = "BB";
    CHECK(SV_reverse_find(SV_from_terminated(two_byte_needle), 2,
                          SV_from_terminated("BB")),
          0UL, size_t, "%zu");
    CHECK(SV_reverse_find(SV_from_terminated(two_byte_needle), 2,
                          SV_from_terminated("AB")),
          2UL, size_t, "%zu");
    CHECK(SV_reverse_find(SV_from_terminated(two_byte_needle), 2,
                          SV_from_terminated("BA")),
          2UL, size_t, "%zu");
    char const *three_byte_needle = "DCC";
    CHECK(SV_reverse_find(SV_from_terminated(three_byte_needle), 3,
                          SV_from_terminated("DCC")),
          0UL, size_t, "%zu");
    CHECK(SV_reverse_find(SV_from_terminated(three_byte_needle), 3,
                          SV_from_terminated("ACC")),
          3UL, size_t, "%zu");
    CHECK(SV_reverse_find(SV_from_terminated(three_byte_needle), 3,
                          SV_from_terminated("DAC")),
          3UL, size_t, "%zu");
    CHECK(SV_reverse_find(SV_from_terminated(three_byte_needle), 3,
                          SV_from_terminated("DCA")),
          3UL, size_t, "%zu");
    char const *four_byte_needle = "YDDD";
    CHECK(SV_reverse_find(SV_from_terminated(four_byte_needle), 4,
                          SV_from_terminated("YDDD")),
          0UL, size_t, "%zu");
    CHECK(SV_reverse_find(SV_from_terminated(four_byte_needle), 4,
                          SV_from_terminated("ADDD")),
          4UL, size_t, "%zu");
    CHECK(SV_reverse_find(SV_from_terminated(four_byte_needle), 4,
                          SV_from_terminated("YDBD")),
          4UL, size_t, "%zu");
    CHECK(SV_reverse_find(SV_from_terminated(four_byte_needle), 4,
                          SV_from_terminated("YDDA")),
          4UL, size_t, "%zu");
    char const *needle = "Zind the needle!";
    CHECK(SV_reverse_find(SV_from_terminated(needle), strlen(needle),
                          SV_from_terminated(needle)),
          0UL, size_t, "%zu");
    char const *const haystack = "DDDD++CCC+++AB+++A+++find the needle!+++";
    SV_Str_view const haystack_view = SV_from_terminated(haystack);
    size_t const one_byte_pos
        = SV_reverse_find(haystack_view, SV_len(haystack_view),
                          SV_from_terminated(one_byte_needle));
    CHECK(one_byte_pos, SV_len(haystack_view), size_t, "%zu");
    size_t const two_byte_pos
        = SV_reverse_find(haystack_view, SV_len(haystack_view),
                          SV_from_terminated(two_byte_needle));
    CHECK(two_byte_pos, SV_len(haystack_view), size_t, "%zu");
    size_t const three_byte_pos
        = SV_reverse_find(haystack_view, SV_len(haystack_view),
                          SV_from_terminated(three_byte_needle));
    CHECK(three_byte_pos, SV_len(haystack_view), size_t, "%zu");
    size_t const four_byte_pos
        = SV_reverse_find(haystack_view, SV_len(haystack_view),
                          SV_from_terminated(four_byte_needle));
    CHECK(four_byte_pos, SV_len(haystack_view), size_t, "%zu");
    size_t const needle_pos = SV_reverse_find(
        haystack_view, SV_len(haystack_view), SV_from_terminated(needle));
    CHECK(needle_pos, SV_len(haystack_view), size_t, "%zu");
    char const *const haystack2 = "this entire string should be a match";
    char const *const needle2 = "this entire string should be a match";
    CHECK(SV_reverse_find(SV_from_terminated(haystack2), strlen(haystack2),
                          SV_from_terminated(needle2)),
          0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_find_rfind_memoization(void) {
    char const *needle_forward = "aabbaabba";
    char const *needle_backward = "abbaabbaa";
    char const *const haystack
        = "forward border aabbaabba backward border abbaabbaa!";
    char const *forward_found = strstr(haystack, needle_forward);
    char const *backward_found = strstr(haystack, needle_backward);
    SV_Str_view const haystack_view = SV_from_terminated(haystack);

    size_t const forward_needle_pos
        = SV_find(haystack_view, 0, SV_from_terminated(needle_forward));
    CHECK(forward_needle_pos, (size_t)(forward_found - haystack), size_t,
          "%zu");
    size_t const backward_needle_pos
        = SV_reverse_find(haystack_view, SV_len(haystack_view),
                          SV_from_terminated(needle_backward));
    CHECK(backward_needle_pos, (size_t)(backward_found - haystack), size_t,
          "%zu");
    return PASS;
}

static enum Test_result
test_substring_off_by_one(void) {
    char const *needle = "needle";
    size_t const needle_len = strlen(needle);
    char const *const haystack = "needle_haystackhaystackhaystack_needle";
    SV_Str_view const haystack_view = SV_from_terminated(haystack);
    SV_Str_view const needle_view = SV_from_terminated(needle);
    char const *ref = strstr(haystack, needle);
    SV_Str_view const found_first = SV_match(haystack_view, needle_view);
    CHECK(SV_terminated_compare(found_first, needle), SV_ORDER_EQUAL, SV_Order,
          "%d");
    CHECK(SV_begin(found_first), ref, char *const, "%s");

    size_t const find_pos = SV_find(haystack_view, 0, needle_view);
    CHECK(find_pos, (size_t)(ref - haystack), size_t, "%zu");

    char const *ref2 = strstr(haystack + needle_len, needle);
    SV_Str_view const found_second = SV_match(
        SV_substr(haystack_view, needle_len, ULLONG_MAX), needle_view);
    CHECK(SV_begin(found_second), ref2, char *const, "%s");
    CHECK(SV_terminated_compare(found_second, needle), SV_ORDER_EQUAL, SV_Order,
          "%d");

    size_t const find_pos2 = SV_find(
        SV_substr(haystack_view, needle_len, ULLONG_MAX), 0, needle_view);
    CHECK(find_pos2, (size_t)(ref2 - (haystack + needle_len)), size_t, "%zu");
    size_t const find_pos2_rev
        = SV_reverse_find(haystack_view, SV_len(haystack_view), needle_view);
    CHECK((size_t)(ref2 - haystack), find_pos2_rev, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_substring_search(void) {
    char const *needle = "needle";
    size_t const needle_len = strlen(needle);
    char const *const haystack
        = "haystackhaystackhaystackhaystackhaystackhaystackhaystackhaystack"
          "haystackhaystackhaystackhaystackhaystackhaystack--------___---**"
          "haystackhaystackhaystackhaystackhaystackhaystack\n\n\n\n\n\n\n\n"
          "neeedleneeddleneedlaneeeeeeeeeeeeeedlenedlennnnnnnnnneeeedneeddl"
          "_______________________needle___________________________________"
          "neeedleneeddleneedlaneeeeeeeeeeeeeedlenedlennneeeeeeeeeeedneeddl"
          "haystackhaystackhaystackhaystackhaystackhaystackhaystack__needle";
    SV_Str_view const haystack_view = SV_from_terminated(haystack);
    SV_Str_view const needle_view = SV_from_terminated(needle);
    char const *a = strstr(haystack, needle);
    if (!a) {
        printf("clibrary strstr failed?\n");
        return FAIL;
    }

    SV_Str_view b = SV_from_view(needle_len, a);
    SV_Str_view c = SV_match(haystack_view, needle_view);

    CHECK(SV_compare(b, c), SV_ORDER_EQUAL, SV_Order, "%d");
    CHECK(SV_begin(c), a, char *const, "%s");
    a += needle_len;
    a = strstr(a, needle);
    if (!a) {
        printf("clibrary strstr failed?\n");
        return FAIL;
    }
    SV_Str_view const new_haystack_view = SV_from_terminated(a);
    b = SV_from_view(needle_len, a);
    c = SV_match(new_haystack_view, needle_view);
    CHECK(SV_compare(b, c), SV_ORDER_EQUAL, SV_Order, "%d");
    CHECK(SV_begin(c), a, char *const, "%s");
    SV_Str_view const first_chunk
        = SV_substr(haystack_view, 0, SV_find(haystack_view, 0, needle_view));
    SV_Str_view const remaining_string = SV_from_terminated(
        SV_begin(first_chunk) + SV_len(first_chunk) + needle_len);
    SV_Str_view const second_chunk = SV_substr(
        remaining_string, 0, SV_find(remaining_string, 0, needle_view));
    /* There are two needles so we get two string chunks chunks. */
    size_t i = 0;
    for (SV_Str_view v = SV_token_begin(haystack_view, needle_view);
         !SV_token_end(haystack_view, v);
         v = SV_token_next(haystack_view, v, needle_view)) {
        if (i == 0) {
            CHECK(SV_compare(v, first_chunk), SV_ORDER_EQUAL, SV_Order, "%d");
        } else {
            CHECK(SV_compare(v, second_chunk), SV_ORDER_EQUAL, SV_Order, "%d");
        }
        ++i;
    }
    CHECK(i, 2UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_rsubstring_search(void) {
    char const *needle = "needle";
    char const *const haystack
        = "needle___khaystackhaystackhaystackhaystackhaystackhaystackhaystack"
          "haystackhaystackhaystackhaystackhaystackhaystack--------___---**"
          "haystackhaystackhaystackhaystackhaystackhaystack\n\n\n\n\n\n\n\n"
          "neeedleneeddleneedlaneeeeeeeeeeeeeedlenedlennnnnnnnnneeeedneeddl"
          "_______________________needle___________________________________"
          "neeedleneeddleneedlaneeeeeeeeeeeeeedlenedlennneeeeeeeeeeedneeddl"
          "haystackhaystackhaystackhaystackhaystackhaystackhaystack";
    SV_Str_view const haystack_view = SV_from_terminated(haystack);
    SV_Str_view const needle_view = SV_from_terminated(needle);
    char const *middle = strstr(haystack + 1, needle);
    char const *begin = strstr(haystack, needle);
    if (!middle || !begin || begin == middle) {
        printf("clibrary strstr failed?\n");
        return ERROR;
    }
    SV_Str_view const middle_needle
        = SV_reverse_match(haystack_view, needle_view);
    size_t const middle_pos
        = SV_reverse_find(haystack_view, SV_len(haystack_view), needle_view);
    CHECK(SV_compare(middle_needle, needle_view), SV_ORDER_EQUAL, SV_Order,
          "%d");
    CHECK(SV_begin(middle_needle), middle, char *const, "%s");
    CHECK(middle_pos, (size_t)(middle - haystack), size_t, "%zu");
    SV_Str_view const first_chunk_view = SV_from_view(middle_pos, haystack);
    SV_Str_view const begin_needle
        = SV_reverse_match(first_chunk_view, needle_view);
    size_t const begin_pos = SV_reverse_find(
        first_chunk_view, SV_len(first_chunk_view), needle_view);
    CHECK(SV_compare(begin_needle, needle_view), SV_ORDER_EQUAL, SV_Order,
          "%d");
    CHECK(SV_begin(begin_needle), begin, char *const, "%s");
    CHECK(begin_pos, (size_t)(begin - haystack), size_t, "%zu");
    return PASS;
}

static enum Test_result
test_long_substring(void) {
    char const *needle = "This needle will make up most of the string such "
                         "that the two-way string searching algorithm has to "
                         "continue for many iterations during a match.";
    char const *const haystack
        = "Here is the string containing the longer needle. This needle will "
          "make up most of the string such that the two-way string searching "
          "algorithm has to continue for many iterations during a match. There "
          "went the needle.";
    char const *strstr_needle = strstr(haystack, needle);
    SV_Str_view const haystack_view = SV_from_terminated(haystack);
    SV_Str_view const needle_view = SV_from_terminated(needle);
    SV_Str_view const svsv_needle = SV_match(haystack_view, needle_view);
    CHECK(SV_begin(svsv_needle), strstr_needle, char *const, "%s");
    size_t const find_pos = SV_find(haystack_view, 0, needle_view);
    CHECK(find_pos, (size_t)(strstr_needle - haystack), size_t, "%zu");
    SV_Str_view const rsvsv_needle
        = SV_reverse_match(haystack_view, needle_view);
    CHECK(SV_begin(rsvsv_needle), strstr_needle, char *const, "%s");
    size_t const rfind_pos
        = SV_reverse_find(haystack_view, SV_len(haystack_view), needle_view);
    CHECK(rfind_pos, (size_t)(strstr_needle - haystack), size_t, "%zu");
    CHECK(SV_compare(svsv_needle, rsvsv_needle), SV_ORDER_EQUAL, SV_Order,
          "%d");
    return PASS;
}
