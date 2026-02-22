#include "str_view.h"
#include "test.h"

#include <string.h>

static enum Test_result test_front_back_terminated(void);
static enum Test_result test_front_back_view(void);

#define NUM_TESTS (size_t)2

static Test_fn const all_tests[NUM_TESTS] = {
    test_front_back_terminated,
    test_front_back_view,
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
test_front_back_terminated(void) {
    CHECK(SV_back(SV_from_terminated("")), '\0', char, "%c");
    CHECK(SV_front(SV_from_terminated("")), '\0', char, "%c");
    char const *const reference = "*The front was * the back is!";
    SV_Str_view const s = SV_from_terminated(reference);
    size_t const ref_len = strlen(reference);
    CHECK(ref_len, SV_len(s), size_t, "%zu");
    CHECK(SV_front(s), '*', char, "%c");
    CHECK(SV_back(s), '!', char, "%c");
    return PASS;
}

static enum Test_result
test_front_back_view(void) {
    char const reference[20] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '^',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '*',
        [10] = ' ', [11] = '@', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    SV_Str_view const s = SV_from_view(8, reference + 4);
    CHECK(SV_front(s), '^', char, "%c");
    CHECK(SV_back(s), '@', char, "%c");
    CHECK(SV_len(s), 8UL, size_t, "%zu");
    return PASS;
}
