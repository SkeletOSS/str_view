#include "str_view.h"
#include "test.h"

#include <limits.h>
#include <stdbool.h>
#include <string.h>

static SV_Str_view const dirslash = SV_from("/");

static enum Test_result test_prefix_suffix(void);
static enum Test_result test_substr(void);
static enum Test_result test_dir_entries(void);
static enum Test_result test_progressive_search(void);

#define NUM_TESTS (size_t)4
static Test_fn const all_tests[NUM_TESTS] = {
    test_prefix_suffix,
    test_substr,
    test_dir_entries,
    test_progressive_search,
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
test_prefix_suffix(void) {
    char const *const reference = "Remove the suffix! No, remove the prefix!";
    char const *const ref_prefix = "Remove the suffix!";
    char const *const ref_suffix = "No, remove the prefix!";
    SV_Str_view entire_string = SV_from_terminated(reference);
    SV_Str_view prefix = SV_remove_suffix(entire_string, 23);
    size_t i = 0;
    for (char const *c = SV_begin(prefix); c != SV_end(prefix);
         c = SV_next(c)) {
        CHECK(*c, ref_prefix[i], char, "%c");
        ++i;
    }
    i = 0;
    SV_Str_view const suffix = SV_remove_prefix(entire_string, 19);
    for (char const *c = SV_begin(suffix); c != SV_end(suffix);
         c = SV_next(c)) {
        CHECK(*c, ref_suffix[i], char, "%c");
        ++i;
    }
    CHECK(SV_is_empty(SV_remove_prefix(entire_string, ULLONG_MAX)), true, bool,
          "%d");
    CHECK(SV_is_empty(SV_remove_suffix(entire_string, ULLONG_MAX)), true, bool,
          "%d");
    return PASS;
}

static enum Test_result
test_substr(void) {
    char const ref[27] = {
        [0] = 'A',  [1] = ' ',  [2] = 's',   [3] = 'u',  [4] = 'b',  [5] = 's',
        [6] = 't',  [7] = 'r',  [8] = 'i',   [9] = 'n',  [10] = 'g', [11] = '!',
        [12] = ' ', [13] = 'H', [14] = 'a',  [15] = 'v', [16] = 'e', [17] = ' ',
        [18] = 'a', [19] = 'n', [20] = 'o',  [21] = 't', [22] = 'h', [23] = 'e',
        [24] = 'r', [25] = '!', [26] = '\0',
    };
    char const *const substr1 = "A substring!";
    char const *const substr2 = "Have another!";
    SV_Str_view const substr1_view = SV_from_terminated(substr1);
    SV_Str_view const substr2_view = SV_from_terminated(substr2);
    CHECK(SV_terminated_compare(
              SV_substr(SV_from_terminated(ref), 0, strlen(substr1)), substr1),
          SV_ORDER_EQUAL, SV_Order, "%d");
    CHECK(SV_terminated_compare(SV_substr(SV_from_terminated(ref),
                                          strlen(substr1) + 1, strlen(substr2)),
                                substr2),
          SV_ORDER_EQUAL, SV_Order, "%d");
    CHECK(SV_terminated_compare(
              SV_substr(SV_from_terminated(ref), 0, ULLONG_MAX), ref),
          SV_ORDER_EQUAL, SV_Order, "%d");
    /* Make sure the fill function adds null terminator */
    char dump_substr1[27] = {[13] = '@'};
    (void)SV_fill(27, dump_substr1,
                  SV_substr(SV_from_terminated(ref), 0, strlen(substr1)));
    CHECK(SV_terminated_compare(substr1_view, dump_substr1), SV_ORDER_EQUAL,
          SV_Order, "%d");
    /* Make sure the fill function adds null terminator */
    char dump_substr2[27] = {[14] = '@'};
    (void)SV_fill(27, dump_substr2,
                  SV_substr(SV_from_terminated(ref), strlen(substr1) + 1,
                            strlen(substr2)));
    CHECK(SV_terminated_compare(substr2_view, dump_substr2), SV_ORDER_EQUAL,
          SV_Order, "%d");
    return PASS;
}

static enum Test_result
test_dir_entries(void) {
    CHECK(SV_is_empty(
              SV_substr(dirslash, 0, SV_reverse_find(dirslash, 0, dirslash))),
          true, bool, "%d");
    SV_Str_view const root_single_entry = SV_from_terminated("/usr");
    SV_Str_view const root_single_entry_slash = SV_from_terminated("/usr/");
    SV_Str_view const without_last_slash
        = SV_substr(root_single_entry_slash, 0,
                    SV_reverse_find(root_single_entry_slash,
                                    SV_len(root_single_entry_slash), dirslash));
    CHECK(SV_compare(without_last_slash, root_single_entry), SV_ORDER_EQUAL,
          SV_Order, "%d");
    SV_Str_view const special_file
        = SV_from_terminated("/this/is/a/very/special/file");
    char const *const toks[6] = {"this", "is", "a", "very", "special", "file"};
    size_t i = 0;
    for (SV_Str_view tok = SV_token_begin(special_file, dirslash);
         !SV_token_end(special_file, tok);
         tok = SV_token_next(special_file, tok, dirslash), ++i) {
        CHECK(SV_terminated_compare(tok, toks[i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
    }
    CHECK(i, sizeof(toks) / sizeof(toks[0]), size_t, "%zu");
    return PASS;
}

static enum Test_result
test_progressive_search(void) {
    SV_Str_view const starting_path
        = SV_from_terminated("/this/is/not/the/file/you/are/looking/for");
    char const *const sub_paths[10] = {
        "/this/is/not/the/file/you/are/looking/for",
        "this/is/not/the/file/you/are/looking/for",
        "is/not/the/file/you/are/looking/for",
        "not/the/file/you/are/looking/for",
        "the/file/you/are/looking/for",
        "file/you/are/looking/for",
        "you/are/looking/for",
        "are/looking/for",
        "looking/for",
        "for",
    };
    size_t i = 0;
    for (SV_Str_view path = starting_path; !SV_is_empty(path);
         path = SV_remove_prefix(path, SV_find_first_of(path, dirslash) + 1)) {
        CHECK(SV_terminated_compare(path, sub_paths[i]), SV_ORDER_EQUAL,
              SV_Order, "%d");
        ++i;
    }
    CHECK(i, sizeof(sub_paths) / sizeof(sub_paths[0]), size_t, "%zu");
    char const *const sub_paths_rev[9] = {
        "/this/is/not/the/file/you/are/looking/for",
        "/this/is/not/the/file/you/are/looking",
        "/this/is/not/the/file/you/are",
        "/this/is/not/the/file/you",
        "/this/is/not/the/file",
        "/this/is/not/the",
        "/this/is/not",
        "/this/is",
        "/this",
    };
    i = 0;
    for (SV_Str_view path = starting_path; !SV_is_empty(path);
         path = SV_remove_suffix(path, SV_len(path)
                                           - SV_find_last_of(path, dirslash))) {
        CHECK(SV_terminated_compare(path, sub_paths_rev[i]), SV_ORDER_EQUAL,
              SV_Order, "%d");
        ++i;
    }
    CHECK(i, sizeof(sub_paths_rev) / sizeof(sub_paths_rev[0]), size_t, "%zu");
    return PASS;
}
