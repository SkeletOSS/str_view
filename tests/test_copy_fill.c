#include "str_view.h"
#include "test.h"

#include <string.h>

static enum Test_result test_copy_fill(void);
static enum Test_result test_copy_section(void);

#define NUM_TESTS (size_t)2
static Test_fn const all_tests[NUM_TESTS] = {
    test_copy_fill,
    test_copy_section,
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
test_copy_fill(void) {
    char const *const reference = "Copy this over there!";
    SV_Str_view this = SV_copy(strlen(reference), reference);
    char there[SV_str_bytes(reference)];
    CHECK(SV_fill(sizeof there, there, this), sizeof there, size_t, "%zu");
    CHECK(strcmp(SV_begin(this), there), 0, SV_Order, "%d");
    CHECK(strlen(there), SV_len(this), size_t, "%zu");
    CHECK(there[(sizeof there) - 1], '\0', char, "%c");
    return PASS;
}

static enum Test_result
test_copy_section(void) {
    char const ref[20] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '!',
        [5] = 's',  [6] = 'n',  [7] = 'i',  [8] = 'p',  [9] = '!',
        [10] = ' ', [11] = '_', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    char const *const expected_snip = "snip!";
    SV_Str_view const ref_view = SV_from_terminated(ref);
    SV_Str_view const snip = SV_substr(ref_view, 5, 5);
    char snip_buf[SV_bytes(snip)];
    CHECK(SV_fill(sizeof snip_buf, snip_buf, snip), SV_bytes(snip), size_t,
          "%zu");
    CHECK(strcmp(expected_snip, snip_buf), 0, SV_Order, "%d");
    CHECK(snip_buf[(sizeof snip_buf) - 1], '\0', char, "%c");
    return PASS;
}
