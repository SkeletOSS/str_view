#include "str_view.h"
#include "test.h"

#include <string.h>

static enum Test_result test_iter(void);
static enum Test_result test_iter2(void);
static enum Test_result test_riter(void);
static enum Test_result test_riter2(void);
static enum Test_result test_riter_multi(void);
static enum Test_result test_min_delim(void);
static enum Test_result test_min_delim_two_byte(void);
static enum Test_result test_min_delim_three_byte(void);
static enum Test_result test_min_delim_four_byte(void);
static enum Test_result test_min_delim_five_byte(void);
static enum Test_result test_rmin_delim(void);
static enum Test_result test_rmin_delim_two_byte(void);
static enum Test_result test_rmin_delim_three_byte(void);
static enum Test_result test_rmin_delim_four_byte(void);
static enum Test_result test_rmin_delim_five_byte(void);
static enum Test_result test_simple_delim(void);
static enum Test_result test_rsimple_delim(void);
static enum Test_result test_tail_delim(void);
static enum Test_result test_rtail_delim(void);
static enum Test_result test_rtriple_delim(void);
static enum Test_result test_rquad_delim(void);
static enum Test_result test_iter_repeating_delim(void);
static enum Test_result test_iter_multichar_delim(void);
static enum Test_result test_riter_multichar_delim(void);
static enum Test_result test_iter_multichar_delim_short(void);
static enum Test_result test_riter_multichar_delim_short(void);
static enum Test_result test_iter_delim_larger_than_str(void);
static enum Test_result test_riter_delim_larger_than_str(void);
static enum Test_result test_tokenize_not_terminated(void);
static enum Test_result test_tokenize_three_views(void);
static enum Test_result test_rtokenize_three_views(void);

#define NUM_TESTS (size_t)31
static Test_fn const all_tests[NUM_TESTS] = {
    test_iter,
    test_iter2,
    test_riter,
    test_riter_multi,
    test_riter2,
    test_min_delim,
    test_min_delim_two_byte,
    test_min_delim_three_byte,
    test_min_delim_four_byte,
    test_min_delim_five_byte,
    test_rmin_delim,
    test_rmin_delim_two_byte,
    test_rmin_delim_three_byte,
    test_rmin_delim_four_byte,
    test_rmin_delim_five_byte,
    test_simple_delim,
    test_rsimple_delim,
    test_tail_delim,
    test_rtail_delim,
    test_rtriple_delim,
    test_rquad_delim,
    test_iter_repeating_delim,
    test_iter_multichar_delim,
    test_riter_multichar_delim,
    test_iter_multichar_delim_short,
    test_riter_multichar_delim_short,
    test_iter_delim_larger_than_str,
    test_riter_delim_larger_than_str,
    test_tokenize_not_terminated,
    test_tokenize_three_views,
    test_rtokenize_three_views,
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
test_iter(void) {
    char const *const reference = "A B C D E G H I J K L M N O P";
    SV_Str_view chars = SV_from_terminated(reference);
    size_t i = 0;
    for (char const *cur = SV_begin(chars); cur != SV_end(chars);
         cur = SV_next(cur)) {
        CHECK(*cur, reference[i], char, "%c");
        ++i;
    }
    i = 0;
    SV_Str_view cur = SV_token_begin(chars, SV_from(" "));
    for (; !SV_token_end(chars, cur);
         cur = SV_token_next(chars, cur, SV_from(" "))) {
        CHECK(SV_front(cur), reference[i], char, "%c");
        i += 2;
    }
    CHECK(SV_front(cur), '\0', char, "%c");
    SV_Str_view cur2 = SV_token_begin(chars, SV_from(","));
    for (; !SV_token_end(chars, cur2);
         cur2 = SV_token_next(chars, cur2, SV_from(","))) {
        CHECK(SV_terminated_compare(cur2, reference), SV_ORDER_EQUAL, SV_Order,
              "%d");
    }
    CHECK(SV_front(cur2), '\0', char, "%c");
    return PASS;
}

static enum Test_result
test_iter2(void) {
    char const *const reference = " A B C D E G H I J K L M N O P ";
    size_t const size = 15;
    char const *const toks[15] = {
        "A", "B", "C", "D", "E", "G", "H", "I",
        "J", "K", "L", "M", "N", "O", "P",
    };
    SV_Str_view chars = SV_from_terminated(reference);
    size_t i = 0;
    for (char const *cur = SV_begin(chars);
         cur != SV_end(chars) && i < SV_len(chars); cur = SV_next(cur)) {
        CHECK(*cur, reference[i], char, "%c");
        ++i;
    }
    i = 0;
    SV_Str_view cur = SV_token_begin(chars, SV_from(" "));
    for (; !SV_token_end(chars, cur) && i < size;
         cur = SV_token_next(chars, cur, SV_from(" "))) {
        CHECK(SV_front(cur), *toks[i], char, "%c");
        CHECK(SV_len(cur), strlen(toks[i]), size_t, "%zu");
        ++i;
    }
    CHECK(SV_front(cur), '\0', char, "%c");
    i = 0;
    SV_Str_view cur2 = SV_token_begin(chars, SV_from(","));
    for (; !SV_token_end(chars, cur2) && i < 1;
         cur2 = SV_token_next(chars, cur2, SV_from(","))) {
        CHECK(SV_terminated_compare(cur2, reference), SV_ORDER_EQUAL, SV_Order,
              "%d");
        ++i;
    }
    CHECK(SV_front(cur2), '\0', char, "%c");
    return PASS;
}

static enum Test_result
test_riter(void) {
    SV_Str_view const ref = SV_from("A B C D E G H I J K L M N O P");
    size_t i = SV_len(ref) - 1;
    SV_Str_view cur = SV_token_reverse_begin(ref, SV_from(" "));
    for (; !SV_token_reverse_end(ref, cur);
         cur = SV_token_reverse_next(ref, cur, SV_from(" "))) {
        CHECK(SV_front(cur), *SV_pointer(ref, i), char, "%c");
        i -= 2;
    }
    CHECK(SV_begin(cur), SV_begin(ref), char *const, "%s");
    SV_Str_view cur2 = SV_token_reverse_begin(ref, SV_from(","));
    for (; !SV_token_reverse_end(ref, cur2);
         cur2 = SV_token_reverse_next(ref, cur2, SV_from(","))) {
        CHECK(SV_compare(cur2, ref), SV_ORDER_EQUAL, SV_Order, "%d");
    }
    CHECK(SV_begin(cur2), SV_begin(ref), char *const, "%s");
    return PASS;
}

static enum Test_result
test_riter2(void) {
    SV_Str_view const ref = SV_from(" A B C D E G H I J K L M N O P ");
    size_t const size = 15;
    char const *const toks[15] = {
        "A", "B", "C", "D", "E", "G", "H", "I",
        "J", "K", "L", "M", "N", "O", "P",
    };
    size_t character = SV_len(ref);
    for (char const *c = SV_reverse_begin(ref);
         character && c != SV_reverse_end(ref); c = SV_reverse_next(c)) {
        --character;
        CHECK(c, SV_pointer(ref, character), char *const, "%s");
        CHECK(*c, SV_at(ref, character), char, "%c");
    }
    CHECK(character, 0UL, size_t, "%zu");
    size_t i = size;
    SV_Str_view cur = SV_token_reverse_begin(ref, SV_from(" "));
    for (; !SV_token_reverse_end(ref, cur) && i;
         cur = SV_token_reverse_next(ref, cur, SV_from(" "))) {
        --i;
        CHECK(SV_front(cur), *toks[i], char, "%c");
        CHECK(SV_len(cur), strlen(toks[i]), size_t, "%zu");
    }
    CHECK(SV_begin(cur), SV_begin(ref), char *const, "%s");
    i = 1;
    SV_Str_view cur2 = SV_token_reverse_begin(ref, SV_from(","));
    for (; !SV_token_reverse_end(ref, cur2) && i;
         cur2 = SV_token_reverse_next(ref, cur2, SV_from(","))) {
        --i;
        CHECK(SV_compare(cur2, ref), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(cur2), SV_len(ref), size_t, "%zu");
    }
    CHECK(SV_begin(cur2), SV_begin(ref), char *const, "%s");
    return PASS;
}

static enum Test_result
test_riter_multi(void) {
    SV_Str_view const ref
        = SV_from("//A//B//C//D//E//G//H//I//J//K//L//M//N//O//P//");
    SV_Str_view const delim = SV_from("//");
    size_t const size = 15;
    char const *const toks[15] = {
        "A", "B", "C", "D", "E", "G", "H", "I",
        "J", "K", "L", "M", "N", "O", "P",
    };
    size_t i = size;
    size_t const last_delim_pos = SV_reverse_find(ref, SV_len(ref), delim);
    CHECK(last_delim_pos, SV_len(ref) - 2, size_t, "%zu");
    SV_Str_view cur = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, cur) && i;
         cur = SV_token_reverse_next(ref, cur, delim)) {
        --i;
        CHECK(SV_front(cur), *toks[i], char, "%c");
        CHECK(SV_len(cur), strlen(toks[i]), size_t, "%zu");
    }
    CHECK(SV_begin(cur), SV_begin(ref), char *const, "%s");
    i = 1;
    SV_Str_view cur2 = SV_token_reverse_begin(ref, SV_from(","));
    for (; !SV_token_reverse_end(ref, cur2) && i;
         cur2 = SV_token_reverse_next(ref, cur2, SV_from(","))) {
        --i;
        CHECK(SV_compare(cur2, ref), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(cur2), SV_len(ref), size_t, "%zu");
    }
    CHECK(SV_begin(cur2), SV_begin(ref), char *const, "%s");
    return PASS;
}

static enum Test_result
test_min_delim(void) {
    SV_Str_view ref = SV_from("/0");
    SV_Str_view const delim = SV_from("/");
    SV_Str_view const tok = SV_from("0");
    SV_Str_view i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    ref = SV_from("0/");
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    ref = SV_from("/0/");
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    ref = SV_from("0/0");
    size_t sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("/0/0");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("0/0/");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("/0/0/");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_min_delim_two_byte(void) {
    SV_Str_view ref = SV_from("//0");
    SV_Str_view const delim = SV_from("//");
    SV_Str_view const tok = SV_from("0");
    SV_Str_view i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    ref = SV_from("0//");
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    ref = SV_from("//0//");
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    ref = SV_from("0//0");
    size_t sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("//0//0");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("0//0//");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("//0//0//");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_min_delim_three_byte(void) {
    SV_Str_view ref = SV_from("///0");
    SV_Str_view const delim = SV_from("///");
    SV_Str_view const tok = SV_from("0");
    SV_Str_view i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    ref = SV_from("0///");
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    ref = SV_from("///0///");
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    ref = SV_from("0///0");
    size_t sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("///0///0");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("0///0///");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("///0///0///");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_min_delim_four_byte(void) {
    SV_Str_view ref = SV_from("////0");
    SV_Str_view const delim = SV_from("////");
    SV_Str_view const tok = SV_from("0");
    SV_Str_view i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    ref = SV_from("0////");
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    ref = SV_from("////0////");
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    ref = SV_from("0////0");
    size_t sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("////0////0");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("0////0////");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("////0////0////");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_min_delim_five_byte(void) {
    SV_Str_view ref = SV_from("/////0");
    SV_Str_view const delim = SV_from("/////");
    SV_Str_view const tok = SV_from("0");
    SV_Str_view i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    ref = SV_from("0/////");
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    ref = SV_from("/////0/////");
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i); i = SV_token_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    ref = SV_from("0/////0");
    size_t sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("/////0/////0");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("0/////0/////");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("/////0/////0/////");
    sz = 2;
    i = SV_token_begin(ref, delim);
    for (; !SV_token_end(ref, i) && sz; i = SV_token_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_end(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_rmin_delim(void) {
    SV_Str_view ref = SV_from("/0");
    SV_Str_view const delim = SV_from("/");
    SV_Str_view const tok = SV_from("0");
    SV_Str_view i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    ref = SV_from("0/");
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    ref = SV_from("/0/");
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    ref = SV_from("0/0");
    size_t sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("/0/0");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("0/0/");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("/0/0/");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_rmin_delim_two_byte(void) {
    SV_Str_view ref = SV_from("//0");
    SV_Str_view const delim = SV_from("//");
    SV_Str_view const tok = SV_from("0");
    SV_Str_view i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    ref = SV_from("0//");
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    ref = SV_from("//0//");
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    ref = SV_from("0//0");
    size_t sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("//0//0");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("0//0//");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("//0//0//");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_rmin_delim_three_byte(void) {
    SV_Str_view ref = SV_from("///0");
    SV_Str_view const delim = SV_from("///");
    SV_Str_view const tok = SV_from("0");
    SV_Str_view i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    ref = SV_from("0///");
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    ref = SV_from("///0///");
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    ref = SV_from("0///0");
    size_t sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("///0///0");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("0///0///");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("///0///0///");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_rmin_delim_four_byte(void) {
    SV_Str_view ref = SV_from("////0");
    SV_Str_view const delim = SV_from("////");
    SV_Str_view const tok = SV_from("0");
    SV_Str_view i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    ref = SV_from("0////");
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    ref = SV_from("////0////");
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    ref = SV_from("0////0");
    size_t sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("////0////0");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("0////0////");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("////0////0////");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_rmin_delim_five_byte(void) {
    SV_Str_view ref = SV_from("/////0");
    SV_Str_view const delim = SV_from("/////");
    SV_Str_view const tok = SV_from("0");
    SV_Str_view i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    ref = SV_from("0/////");
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    ref = SV_from("/////0/////");
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i);
         i = SV_token_reverse_next(ref, i, delim)) {
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    ref = SV_from("0/////0");
    size_t sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("/////0/////0");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("0/////0/////");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    ref = SV_from("/////0/////0/////");
    sz = 2;
    i = SV_token_reverse_begin(ref, delim);
    for (; !SV_token_reverse_end(ref, i) && sz;
         i = SV_token_reverse_next(ref, i, delim)) {
        --sz;
        CHECK(SV_compare(i, tok), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(i), SV_len(tok), size_t, "%zu");
    }
    CHECK(SV_begin(i), SV_begin(ref), char *const, "%s");
    CHECK(sz, 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_simple_delim(void) {
    char const *const reference = "0/1/2/2/3//3////3/4/4/4/////4";
    char const *const toks[11] = {
        "0", "1", "2", "2", "3", "3", "3", "4", "4", "4", "4",
    };
    SV_Str_view const ref_view = SV_from_terminated(reference);
    SV_Str_view const delim = SV_from("/");
    size_t i = 0;
    for (SV_Str_view tok = SV_token_begin(ref_view, delim);
         !SV_token_end(ref_view, tok) && i < sizeof(toks) / sizeof(toks[0]);
         tok = SV_token_next(ref_view, tok, delim)) {
        CHECK(SV_terminated_compare(tok, toks[i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(tok), strlen(toks[i]), size_t, "%zu");
        ++i;
    }
    CHECK(i, sizeof(toks) / sizeof(toks[0]), size_t, "%zu");
    return PASS;
}

static enum Test_result
test_rsimple_delim(void) {
    char const *const reference = "0/1/2/2/3//3////3/4/4/4/////4";
    char const *const toks[11] = {
        "0", "1", "2", "2", "3", "3", "3", "4", "4", "4", "4",
    };
    size_t const size = sizeof(toks) / sizeof(toks[0]);
    SV_Str_view const ref_view = SV_from_terminated(reference);
    SV_Str_view const delim = SV_from("/");
    size_t i = size;
    for (SV_Str_view tok = SV_token_reverse_begin(ref_view, delim);
         !SV_token_reverse_end(ref_view, tok) && i;
         tok = SV_token_reverse_next(ref_view, tok, delim)) {
        --i;
        CHECK(SV_terminated_compare(tok, toks[i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(tok), strlen(toks[i]), size_t, "%zu");
    }
    CHECK(i, 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_tail_delim(void) {
    char const *const reference = "0/1//2//2//3//3////3//4//4//4///////4578";
    char const *const toks[10] = {
        "0/1", "2", "2", "3", "3", "3", "4", "4", "4", "/4578",
    };
    SV_Str_view const ref_view = SV_from_terminated(reference);
    SV_Str_view const delim = SV_from("//");
    size_t i = 0;
    for (SV_Str_view tok = SV_token_begin(ref_view, delim);
         !SV_token_end(ref_view, tok);
         tok = SV_token_next(ref_view, tok, delim)) {
        CHECK(SV_terminated_compare(tok, toks[i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(tok), strlen(toks[i]), size_t, "%zu");
        ++i;
    }
    CHECK(i, sizeof(toks) / sizeof(toks[0]), size_t, "%zu");
    return PASS;
}

static enum Test_result
test_rtail_delim(void) {
    char const *const reference = "0/1//2//2//3//3////3//4//4//4///4578";
    char const *const toks[10] = {
        "0/1", "2", "2", "3", "3", "3", "4", "4", "4/", "4578",
    };
    size_t const size = sizeof(toks) / sizeof(toks[0]);
    SV_Str_view const ref_view = SV_from_terminated(reference);
    SV_Str_view const delim = SV_from("//");
    size_t i = size;
    for (SV_Str_view tok = SV_token_reverse_begin(ref_view, delim);
         !SV_token_reverse_end(ref_view, tok);
         tok = SV_token_reverse_next(ref_view, tok, delim)) {
        --i;
        CHECK(SV_terminated_compare(tok, toks[i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(tok), strlen(toks[i]), size_t, "%zu");
    }
    CHECK(i, 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_rtriple_delim(void) {
    char const *const reference
        = "!!0/1!!!2!!!2!!!3!3!!!!!!3!!!4!!!4!!4!!!4578";
    char const *const toks[8] = {
        "!!0/1", "2", "2", "3!3", "3", "4", "4!!4", "4578",
    };
    size_t const size = sizeof(toks) / sizeof(toks[0]);
    SV_Str_view const ref_view = SV_from_terminated(reference);
    SV_Str_view const delim = SV_from("!!!");
    size_t i = size;
    for (SV_Str_view tok = SV_token_reverse_begin(ref_view, delim);
         !SV_token_reverse_end(ref_view, tok);
         tok = SV_token_reverse_next(ref_view, tok, delim)) {
        --i;
        CHECK(SV_terminated_compare(tok, toks[i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(tok), strlen(toks[i]), size_t, "%zu");
    }
    CHECK(i, 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_rquad_delim(void) {
    char const *const reference
        = "!!!0/1!!!!2!!!!2!!!!3!!3!!!!!!!!3!!!!4!!!!4!!4!!!!4578";
    char const *const toks[8] = {
        "!!!0/1", "2", "2", "3!!3", "3", "4", "4!!4", "4578",
    };
    size_t const size = sizeof(toks) / sizeof(toks[0]);
    SV_Str_view const ref_view = SV_from_terminated(reference);
    SV_Str_view const delim = SV_from("!!!!");
    size_t i = size;
    for (SV_Str_view tok = SV_token_reverse_begin(ref_view, delim);
         !SV_token_reverse_end(ref_view, tok);
         tok = SV_token_reverse_next(ref_view, tok, delim)) {
        --i;
        CHECK(SV_terminated_compare(tok, toks[i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(tok), strlen(toks[i]), size_t, "%zu");
    }
    CHECK(i, 0UL, size_t, "%zu");
    return PASS;
}

static enum Test_result
test_iter_repeating_delim(void) {
    char const *toks[14] = {
        "A",  "B", "C", "D",   "E", "F",  "G",
        "HI", "J", "K", "LMN", "O", "Pi", "\\(*.*)/",
    };
    char const *const reference
        = " A   B  C     D  E F G HI J   K LMN O   Pi  \\(*.*)/  ";
    SV_Str_view const ref_view = SV_from_terminated(reference);
    size_t i = 0;
    SV_Str_view cur = SV_token_begin(ref_view, SV_from(" "));
    for (; !SV_token_end(ref_view, cur);
         cur = SV_token_next(ref_view, cur, SV_from(" "))) {
        CHECK(SV_terminated_compare(cur, toks[i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(cur), strlen(toks[i]), size_t, "%zu");
        ++i;
    }
    CHECK(SV_front(cur), '\0', char, "%c");
    SV_Str_view cur2 = SV_token_begin(ref_view, SV_from(","));
    for (; !SV_token_end(ref_view, cur2);
         cur2 = SV_token_next(ref_view, cur2, SV_from(","))) {
        CHECK(SV_terminated_compare(cur2, reference), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(cur2), strlen(reference), size_t, "%zu");
    }
    CHECK(SV_front(cur2), '\0', char, "%c");
    return PASS;
}

static enum Test_result
test_iter_multichar_delim(void) {
    char const *toks[14] = {
        "A",     "B", "C", "D",      "E", "F",  "G",
        "HacbI", "J", "K", "LcbaMN", "O", "Pi", "\\(*.*)/",
    };
    char const *const reference
        = "abcAabcBabcCabcabcabcDabcEabcFabcGabcHacbIabcJabcabcabcabcKabcLcbaMN"
          "abcOabcabcPiabcabc\\(*.*)/abc";
    size_t i = 0;
    SV_Str_view const delim = SV_from("abc");
    SV_Str_view const ref_view = SV_from_terminated(reference);
    SV_Str_view cur = SV_token_begin(ref_view, delim);
    for (; !SV_token_end(ref_view, cur);
         cur = SV_token_next(ref_view, cur, delim)) {
        CHECK(SV_terminated_compare(cur, toks[i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(cur), strlen(toks[i]), size_t, "%zu");
        ++i;
    }
    CHECK(SV_front(cur), '\0', char, "%c");
    SV_Str_view cur2 = SV_token_begin(ref_view, SV_from(" "));
    for (; !SV_token_end(ref_view, cur2);
         cur2 = SV_token_next(ref_view, cur2, SV_from(" "))) {
        CHECK(SV_terminated_compare(cur2, reference), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(cur2), strlen(reference), size_t, "%zu");
    }
    CHECK(SV_front(cur2), '\0', char, "%c");
    return PASS;
}

static enum Test_result
test_riter_multichar_delim(void) {
    char const *toks[14] = {
        "A",     "B", "C", "D",      "E", "F",  "G",
        "HacbI", "J", "K", "LcbaMN", "O", "Pi", "\\(*.*)/",
    };
    size_t const size = sizeof(toks) / sizeof(toks[0]);
    char const *const reference
        = "abcAabcBabcCabcabcabcDabcEabcFabcGabcHacbIabcJabcabcabcabcKabcLcbaMN"
          "abcOabcabcPiabcabc\\(*.*)/abc";
    SV_Str_view const delim = SV_from("abc");
    SV_Str_view const ref_view = SV_from_terminated(reference);
    SV_Str_view cur = SV_token_reverse_begin(ref_view, delim);
    size_t i = size;
    for (; !SV_token_reverse_end(ref_view, cur) && i;
         cur = SV_token_reverse_next(ref_view, cur, delim)) {
        --i;
        CHECK(SV_terminated_compare(cur, toks[i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(cur), strlen(toks[i]), size_t, "%zu");
    }
    CHECK(i, 0UL, size_t, "%zu");
    CHECK(SV_begin(cur), reference, char *const, "%s");
    SV_Str_view cur2 = SV_token_reverse_begin(ref_view, SV_from(" "));
    for (; !SV_token_reverse_end(ref_view, cur2);
         cur2 = SV_token_reverse_next(ref_view, cur2, SV_from(" "))) {
        CHECK(SV_terminated_compare(cur2, reference), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(cur2), strlen(reference), size_t, "%zu");
    }
    CHECK(SV_begin(cur2), reference, char *const, "%s");
    return PASS;
}

static enum Test_result
test_iter_multichar_delim_short(void) {
    char const *toks[14] = {
        "A",     "B", "C", "D",      "E",   "F",  "G",
        "H---I", "J", "K", "L-M--N", "--O", "Pi", "\\(*.*)/",
    };
    char const *const reference = "-----A-----B-----C-----D-----E-----F-----G--"
                                  "---H---I-----J-----K-----L-M--N"
                                  "-------O-----Pi-----\\(*.*)/-----";
    size_t i = 0;
    SV_Str_view const delim = SV_from("-----");
    SV_Str_view const ref_view = SV_from_terminated(reference);
    SV_Str_view cur = SV_token_begin(ref_view, delim);
    for (; !SV_token_end(ref_view, cur);
         cur = SV_token_next(ref_view, cur, delim)) {
        CHECK(SV_terminated_compare(cur, toks[i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(cur), strlen(toks[i]), size_t, "%zu");
        ++i;
    }
    CHECK(SV_front(cur), '\0', char, "%c");
    SV_Str_view cur2 = SV_token_begin(ref_view, SV_from(" "));
    for (; !SV_token_end(ref_view, cur2);
         cur2 = SV_token_next(ref_view, cur2, SV_from(" "))) {
        CHECK(SV_terminated_compare(cur2, reference), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(cur2), strlen(reference), size_t, "%zu");
    }
    CHECK(SV_front(cur2), '\0', char, "%c");
    return PASS;
}

static enum Test_result
test_riter_multichar_delim_short(void) {
    char const *toks[14] = {
        "A",     "B", "C", "D",        "E", "F",  "G",
        "H---I", "J", "K", "L-M--N--", "O", "Pi", "\\(*.*)/",
    };
    size_t const size = sizeof(toks) / sizeof(toks[0]);
    char const *const reference = "-----A-----B-----C-----D-----E-----F-----G--"
                                  "---H---I-----J-----K-----L-M--N"
                                  "-------O-----Pi-----\\(*.*)/-----";
    size_t i = size;
    SV_Str_view const delim = SV_from("-----");
    SV_Str_view const ref_view = SV_from_terminated(reference);
    SV_Str_view cur = SV_token_reverse_begin(ref_view, delim);
    for (; !SV_token_reverse_end(ref_view, cur);
         cur = SV_token_reverse_next(ref_view, cur, delim)) {
        --i;
        CHECK(SV_terminated_compare(cur, toks[i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(cur), strlen(toks[i]), size_t, "%zu");
    }
    CHECK(SV_begin(cur), reference, char *const, "%s");
    CHECK(i, 0UL, size_t, "%zu");
    SV_Str_view cur2 = SV_token_reverse_begin(ref_view, SV_from(" "));
    for (; !SV_token_reverse_end(ref_view, cur2);
         cur2 = SV_token_reverse_next(ref_view, cur2, SV_from(" "))) {
        CHECK(SV_terminated_compare(cur2, reference), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(cur2), strlen(reference), size_t, "%zu");
    }
    CHECK(SV_begin(cur2), reference, char *const, "%s");
    return PASS;
}

static enum Test_result
test_iter_delim_larger_than_str(void) {
    char const *const ref = "A-B";
    char const *const delim = "-----";
    SV_Str_view const delim_view = SV_from_terminated(delim);
    SV_Str_view const ref_view = SV_from_terminated(ref);
    SV_Str_view constructed = SV_from_delimiter(ref, delim);
    SV_Str_view cur = SV_token_begin(ref_view, delim_view);
    CHECK(SV_compare(constructed, cur), SV_ORDER_EQUAL, SV_Order, "%d");
    CHECK(SV_terminated_compare(constructed, ref), SV_ORDER_EQUAL, SV_Order,
          "%d");
    CHECK(SV_terminated_compare(cur, ref), SV_ORDER_EQUAL, SV_Order, "%d");

    for (; !SV_token_end(ref_view, cur);
         cur = SV_token_next(ref_view, cur, delim_view)) {
        CHECK(SV_terminated_compare(cur, ref), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(cur), strlen(ref), size_t, "%zu");
    }
    CHECK(SV_front(cur), '\0', char, "%c");
    return PASS;
}

static enum Test_result
test_riter_delim_larger_than_str(void) {
    char const *const ref = "A-B";
    char const *const delim = "-----";
    SV_Str_view const delim_view = SV_from_terminated(delim);
    SV_Str_view const ref_view = SV_from_terminated(ref);
    SV_Str_view constructed = SV_from_delimiter(ref, delim);
    SV_Str_view cur = SV_token_reverse_begin(ref_view, delim_view);
    CHECK(SV_compare(constructed, cur), SV_ORDER_EQUAL, SV_Order, "%d");
    CHECK(SV_terminated_compare(constructed, ref), SV_ORDER_EQUAL, SV_Order,
          "%d");
    CHECK(SV_terminated_compare(cur, ref), SV_ORDER_EQUAL, SV_Order, "%d");

    for (; !SV_token_reverse_end(ref_view, cur);
         cur = SV_token_reverse_next(ref_view, cur, delim_view)) {
        CHECK(SV_compare(cur, ref_view), SV_ORDER_EQUAL, SV_Order, "%d");
        CHECK(SV_len(cur), SV_len(ref_view), size_t, "%zu");
    }
    CHECK(*SV_begin(cur), *ref, char, "%c");
    return PASS;
}

static enum Test_result
test_tokenize_not_terminated(void) {
    char const *const path_str = "this/path/will/be/missing/its/child";
    char const *const toks[6] = {
        "this", "path", "will", "be", "missing", "its",
    };
    SV_Str_view const path = SV_from_terminated(path_str);
    SV_Str_view const delim = SV_from("/");
    SV_Str_view const childless_path
        = SV_remove_suffix(path, SV_len(path) - SV_find_last_of(path, delim));
    size_t i = 0;
    for (SV_Str_view tok = SV_token_begin(childless_path, delim);
         !SV_token_end(childless_path, tok);
         tok = SV_token_next(childless_path, tok, delim)) {
        CHECK(SV_terminated_compare(tok, toks[i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(tok), strlen(toks[i]), size_t, "%zu");
        ++i;
    }
    CHECK(i, sizeof(toks) / sizeof(toks[0]), size_t, "%zu");
    return PASS;
}

static enum Test_result
test_tokenize_three_views(void) {
    char const *const path_str = "all/of/these/paths/are/unique/and/split/up";
    char const *const toks[3][3] = {
        {"all", "of", "these"},
        {"paths", "are", "unique"},
        {"and", "split", "up"},
    };
    size_t const size = sizeof(toks) / sizeof(toks[0]);
    SV_Str_view const path = SV_from_terminated(path_str);
    SV_Str_view const delim = SV_from("/");
    SV_Str_view const first
        = SV_substr(path, 0, SV_find(path, 0, SV_from("/paths/")));
    SV_Str_view const second
        = SV_substr(path, SV_find(path, 0, SV_from("/paths/")),
                    SV_find(path, 0, SV_from("/and/"))
                        - SV_find(path, 0, SV_from("/paths/")));
    SV_Str_view const third
        = SV_substr(path, SV_find(path, 0, SV_from("/and/")),
                    SV_len(path) - SV_find(path, 0, SV_from("/and/")));
    size_t i = 0;
    for (SV_Str_view tok1 = SV_token_begin(first, delim),
                     tok2 = SV_token_begin(second, delim),
                     tok3 = SV_token_begin(third, delim);
         !SV_token_end(first, tok1) && !SV_token_end(second, tok2)
         && !SV_token_end(third, tok3) && i < size;
         tok1 = SV_token_next(first, tok1, delim),
                     tok2 = SV_token_next(second, tok2, delim),
                     tok3 = SV_token_next(third, tok3, delim)) {
        CHECK(SV_terminated_compare(tok1, toks[0][i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(tok1), strlen(toks[0][i]), size_t, "%zu");
        CHECK(SV_terminated_compare(tok2, toks[1][i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(tok2), strlen(toks[1][i]), size_t, "%zu");
        CHECK(SV_terminated_compare(tok3, toks[2][i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(tok3), strlen(toks[2][i]), size_t, "%zu");
        ++i;
    }
    CHECK(i, sizeof(toks) / sizeof(toks[0]), size_t, "%zu");
    return PASS;
}

static enum Test_result
test_rtokenize_three_views(void) {
    char const *const path_str = "all/of/these/paths/are/unique/and/split/up";
    char const *const toks[3][3] = {
        {"all", "of", "these"},
        {"paths", "are", "unique"},
        {"and", "split", "up"},
    };
    size_t const size = sizeof(toks) / sizeof(toks[0]);
    SV_Str_view const path = SV_from_terminated(path_str);
    SV_Str_view const delim = SV_from("/");
    SV_Str_view const first
        = SV_substr(path, 0, SV_find(path, 0, SV_from("/paths/")));
    SV_Str_view const second
        = SV_substr(path, SV_find(path, 0, SV_from("/paths/")),
                    SV_find(path, 0, SV_from("/and/"))
                        - SV_find(path, 0, SV_from("/paths/")));
    SV_Str_view const third
        = SV_substr(path, SV_find(path, 0, SV_from("/and/")),
                    SV_len(path) - SV_find(path, 0, SV_from("/and/")));
    size_t i = size;
    for (SV_Str_view tok1 = SV_token_reverse_begin(first, delim),
                     tok2 = SV_token_reverse_begin(second, delim),
                     tok3 = SV_token_reverse_begin(third, delim);
         !SV_token_reverse_end(first, tok1)
         && !SV_token_reverse_end(second, tok2)
         && !SV_token_reverse_end(third, tok3) && i;
         tok1 = SV_token_reverse_next(first, tok1, delim),
                     tok2 = SV_token_reverse_next(second, tok2, delim),
                     tok3 = SV_token_reverse_next(third, tok3, delim)) {
        --i;
        CHECK(SV_terminated_compare(tok1, toks[0][i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(tok1), strlen(toks[0][i]), size_t, "%zu");
        CHECK(SV_terminated_compare(tok2, toks[1][i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(tok2), strlen(toks[1][i]), size_t, "%zu");
        CHECK(SV_terminated_compare(tok3, toks[2][i]), SV_ORDER_EQUAL, SV_Order,
              "%d");
        CHECK(SV_len(tok3), strlen(toks[2][i]), size_t, "%zu");
    }
    CHECK(i, 0UL, size_t, "%zu");
    return PASS;
}
