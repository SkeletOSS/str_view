#include "str_view.h"
#include "test.h"

#include <stdbool.h>
#include <string.h>

static enum Test_result test_compare_single(void);
static enum Test_result test_compare_equal(void);
static enum Test_result test_compare_equal_view(void);
static enum Test_result test_compare_view_equals_str(void);
static enum Test_result test_compare_view_off_by_one(void);
static enum Test_result test_compare_terminated(void);
static enum Test_result test_compare_different_lengths_terminated(void);
static enum Test_result test_compare_different_lengths_views(void);
static enum Test_result test_compare_misc(void);

#define NUM_TESTS (size_t)9
static Test_fn const all_tests[NUM_TESTS] = {
    test_compare_single,
    test_compare_equal,
    test_compare_equal_view,
    test_compare_terminated,
    test_compare_different_lengths_terminated,
    test_compare_view_equals_str,
    test_compare_view_off_by_one,
    test_compare_different_lengths_views,
    test_compare_misc,
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
test_compare_single(void) {
    char const e1[2] = {
        [0] = 'A',
        [1] = '\0',
    };
    char const e2[2] = {
        [0] = 'B',
        [1] = '\0',
    };
    SV_Str_view const e1_view = SV_from_terminated(e1);
    SV_Str_view const e2_view = SV_from_terminated(e2);
    int const cmp_res = strcmp(e1, e2);
    int const cmp_res2 = strcmp(e2, e1);
    CHECK(cmp_res < 0, SV_terminated_compare(e1_view, e2) < 0, bool, "%d");
    CHECK(cmp_res < 0, SV_compare(e1_view, e2_view) < 0, bool, "%d");
    CHECK(cmp_res2 > 0, SV_terminated_compare(e2_view, e1) > 0, bool, "%d");
    CHECK(cmp_res2 > 0, SV_compare(e2_view, e1_view) > 0, bool, "%d");
    return PASS;
}

static enum Test_result
test_compare_equal(void) {
    char const e1[5] = {
        [0] = 'N', [1] = 'I', [2] = 'C', [3] = 'E', [4] = '\0',
    };
    char const e2[5] = {
        [0] = 'N', [1] = 'I', [2] = 'C', [3] = 'E', [4] = '\0',
    };
    SV_Str_view const e1_view = SV_from_terminated(e1);
    SV_Str_view const e2_view = SV_from_terminated(e2);
    int const cmp_res = strcmp(e1, e2);
    int const cmp_res2 = strcmp(e2, e1);
    CHECK(cmp_res == 0, SV_terminated_compare(e1_view, e2) == 0, bool, "%d");
    CHECK(cmp_res == 0, SV_compare(e1_view, e2_view) == 0, bool, "%d");
    CHECK(cmp_res2 == 0, SV_terminated_compare(e2_view, e1) == 0, bool, "%d");
    CHECK(cmp_res2 == 0, SV_compare(e2_view, e1_view) == 0, bool, "%d");
    return PASS;
}

static enum Test_result
test_compare_equal_view(void) {
    char const e1[5] = {
        [0] = 'N', [1] = 'I', [2] = 'C', [3] = 'E', [4] = '\0',
    };
    char const e2[9] = {
        [0] = 'N', [1] = 'I', [2] = 'C', [3] = 'E',  [4] = 'N',
        [5] = 'E', [6] = 'S', [7] = 'S', [8] = '\0',
    };
    SV_Str_view const e1_view = SV_from_terminated(e1);
    SV_Str_view const e2_view = SV_from_view(strlen(e1), e2);
    int const cmp_res = strcmp(e1, e1);
    CHECK(cmp_res == 0, SV_compare(e1_view, e2_view) == 0, bool, "%d");
    CHECK(cmp_res == 0, SV_terminated_compare(e2_view, e1) == 0, bool, "%d");
    CHECK(cmp_res == 0, SV_compare(e2_view, e1_view) == 0, bool, "%d");
    return PASS;
}

static enum Test_result
test_compare_terminated(void) {
    char const lesser[5] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'A', [4] = '\0',
    };
    char const greater[5] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'B', [4] = '\0',
    };
    SV_Str_view const lesser_view = SV_from_terminated(lesser);
    SV_Str_view const greater_view = SV_from_terminated(greater);
    int const cmp_res = strcmp(lesser, greater);
    int const cmp_res2 = strcmp(greater, lesser);
    CHECK(cmp_res < 0, SV_terminated_compare(lesser_view, greater) < 0, bool,
          "%d");
    CHECK(cmp_res < 0, SV_compare(lesser_view, greater_view) < 0, bool, "%d");
    CHECK(cmp_res2 > 0, SV_terminated_compare(greater_view, lesser) > 0, bool,
          "%d");
    CHECK(cmp_res2 > 0, SV_compare(greater_view, lesser_view) > 0, bool, "%d");
    return PASS;
}

static enum Test_result
test_compare_different_lengths_terminated(void) {
    char const lesser[5]
        = {[0] = 'A', [1] = 'A', [2] = 'A', [3] = 'A', [4] = '\0'};
    char const greater[3] = {[0] = 'A', [1] = 'A', [2] = '\0'};
    SV_Str_view const less_view = SV_from_terminated(lesser);
    SV_Str_view const greater_view = SV_from_terminated(greater);
    int const cmp_res = strcmp(lesser, greater);
    int const cmp_res2 = strcmp(greater, lesser);
    CHECK(cmp_res < 0, SV_terminated_compare(less_view, greater) < 0, bool,
          "%d");
    CHECK(cmp_res < 0, SV_compare(less_view, greater_view) < 0, bool, "%d");
    CHECK(cmp_res2 > 0, SV_terminated_compare(greater_view, lesser) > 0, bool,
          "%d");
    CHECK(cmp_res2 > 0, SV_compare(greater_view, less_view) > 0, bool, "%d");
    return PASS;
}

static enum Test_result
test_compare_view_equals_str(void) {
    char const *const views
        = "this string constains substring1, substring2, and substring3";
    char const *const str1 = "substring1";
    char const *const str2 = "substring2";
    char const *const str3 = "substring3";
    SV_Str_view const s1
        = SV_match(SV_from_terminated(views), SV_from_terminated("substring1"));
    SV_Str_view const s2
        = SV_match(SV_from_terminated(views), SV_from_terminated("substring2"));
    SV_Str_view const s3 = SV_reverse_match(SV_from_terminated(views),
                                            SV_from_terminated("substring3"));
    CHECK(SV_terminated_compare(s1, str1), SV_ORDER_EQUAL, SV_Order, "%d");
    CHECK(SV_terminated_compare(s2, str2), SV_ORDER_EQUAL, SV_Order, "%d");
    CHECK(SV_terminated_compare(s3, str3), SV_ORDER_EQUAL, SV_Order, "%d");
    int const cmp_1_2 = strcmp(str1, str2);
    CHECK(SV_terminated_compare(s1, str2) < 0, cmp_1_2 < 0, bool, "%d");
    int const cmp_2_3 = strcmp(str2, str3);
    CHECK(SV_terminated_compare(s2, str3) < 0, cmp_2_3 < 0, bool, "%d");
    int const cmp_3_1 = strcmp(str3, str1);
    CHECK(SV_terminated_compare(s3, str1) > 0, cmp_3_1 > 0, bool, "%d");
    return PASS;
}

static enum Test_result
test_compare_view_off_by_one(void) {
    char const *const views
        = "this string constains substring12, substring2, and substring";
    /* Three views of interest withing the string. */
    SV_Str_view const v1 = SV_match(SV_from_terminated(views),
                                    SV_from_terminated("substring12"));
    SV_Str_view const v2
        = SV_match(SV_from_terminated(views), SV_from_terminated("substring2"));
    SV_Str_view const v3 = SV_reverse_match(SV_from_terminated(views),
                                            SV_from_terminated("substring"));
    /* The null terminated version of those three views. */
    char const *const s1 = "substring12";
    char const *const s2 = "substring2";
    char const *const s3 = "substring";
    /* These strings will not match the substrings within the string. */
    char const *const str1 = "substring1";
    char const *const str2 = "substring22";
    char const *const str3 = "substring3";
    int const cmp_1_1 = strcmp(s1, str1);
    CHECK(SV_terminated_compare(v1, str1) > 0, cmp_1_1 > 0, bool, "%d");
    int const cmp_2_2 = strcmp(s2, str2);
    CHECK(SV_terminated_compare(v2, str2) < 0, cmp_2_2 < 0, bool, "%d");
    int const cmp_3_3 = strcmp(s3, str3);
    CHECK(SV_terminated_compare(v3, str3) < 0, cmp_3_3 < 0, bool, "%d");
    /* Now do the same test but the off by one mismatch is other way. */
    char const *const str1_v2 = "substring121";
    char const *const str2_v2 = "substring";
    char const *const str3_v2 = "substrin";
    int const cmp_1_1_v2 = strcmp(s1, str1_v2);
    CHECK(SV_terminated_compare(v1, str1_v2) < 0, cmp_1_1_v2 < 0, bool, "%d");
    int const cmp_2_2_v2 = strcmp(s2, str2_v2);
    CHECK(SV_terminated_compare(v2, str2_v2) > 0, cmp_2_2_v2 > 0, bool, "%d");
    int const cmp_3_3_v2 = strcmp(s3, str3_v2);
    CHECK(SV_terminated_compare(v3, str3_v2) > 0, cmp_3_3_v2 > 0, bool, "%d");
    return PASS;
}

static enum Test_result
test_compare_different_lengths_views(void) {

    char const lesser[5] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'A', [4] = '\0',
    };
    char const greater_str[5] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'B', [4] = '\0',
    };
    char const greater_longer[9] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'B',  [4] = 'A',
        [5] = 'B', [6] = 'Y', [7] = 'Z', [8] = '\0',
    };
    int const str_cmp = strcmp(lesser, greater_str);
    int const str_cmp2 = strcmp(greater_str, lesser);
    SV_Str_view const greater_view
        = SV_from_view(strlen(greater_str), greater_longer);
    SV_Str_view const lesser_view = SV_from_terminated(lesser);
    CHECK(str_cmp2 > 0, SV_terminated_compare(greater_view, lesser) > 0, bool,
          "%d");
    CHECK(str_cmp < 0, SV_terminated_compare(lesser_view, greater_str) < 0,
          bool, "%d");
    CHECK(str_cmp < 0, SV_compare(lesser_view, greater_view) < 0, bool, "%d");
    CHECK(str_cmp2 > 0, SV_compare(greater_view, lesser_view) > 0, bool, "%d");
    return PASS;
}

static enum Test_result
test_compare_misc(void) {
    CHECK(SV_compare(SV_from_terminated(""), SV_from_terminated("")),
          SV_ORDER_EQUAL, SV_Order, "%d");
    CHECK(SV_terminated_compare(SV_from_terminated(""), ""), SV_ORDER_EQUAL,
          SV_Order, "%d");
    CHECK(SV_compare(SV_from_terminated("same"), SV_from_terminated("same")),
          SV_ORDER_EQUAL, SV_Order, "%d");
    CHECK(SV_compare(SV_from_terminated("samz"), SV_from_terminated("same")),
          SV_ORDER_GREATER, SV_Order, "%d");
    CHECK(SV_compare(SV_from_terminated("same"), SV_from_terminated("samz")),
          SV_ORDER_LESSER, SV_Order, "%d");
    /* The comparison function should treat the end of a string view as
       null terminating character even if it points to a delimeter */
    CHECK(SV_compare(SV_from_terminated("same"),
                     SV_from_delimiter("same same", " ")),
          SV_ORDER_EQUAL, SV_Order, "%d");
    CHECK(SV_compare(SV_from_terminated("same"),
                     SV_from_delimiter("samz same", " ")),
          SV_ORDER_LESSER, SV_Order, "%d");
    CHECK(SV_compare(SV_from_delimiter("sameez same", " "),
                     SV_from_terminated("same")),
          SV_ORDER_GREATER, SV_Order, "%d");
    char const *const str = "same";
    CHECK(SV_terminated_compare(SV_from_terminated(str), str), SV_ORDER_EQUAL,
          SV_Order, "%d");
    CHECK(SV_terminated_compare(SV_from_delimiter("same same", " "), str),
          SV_ORDER_EQUAL, SV_Order, "%d");
    CHECK(SV_terminated_compare(SV_from_delimiter("samez same", " "), str),
          SV_ORDER_GREATER, SV_Order, "%d");
    CHECK(SV_terminated_compare(SV_from_delimiter("sameez same", " "), str),
          SV_ORDER_GREATER, SV_Order, "%d");
    /* strncmp compares at most n bytes inclusize or stops at null. */
    CHECK(SV_view_compare(SV_from_delimiter("sameez same", " "), str, 10),
          SV_ORDER_GREATER, SV_Order, "%d");
    CHECK(SV_view_compare(SV_from_delimiter("saaeez same", " "), str, 3),
          SV_ORDER_LESSER, SV_Order, "%d");
    return PASS;
}
