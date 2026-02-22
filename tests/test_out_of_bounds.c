#include "str_view.h"
#include "test.h"

#include <stddef.h>

int
main(void) {
    SV_Str_view const s = SV_from("a");
    SV_Str_view const empty = SV_from_terminated(SV_pointer(s, 1));
    CHECK(SV_at(s, 9), '\0', char, "%c");
    CHECK(SV_pointer(s, 9), SV_end(s), char *const, "%s");
    SV_Str_view const sub_s = SV_substr(s, 9, 9);
    CHECK(SV_begin(sub_s), SV_begin(empty), char *const, "%s");
    CHECK(SV_len(sub_s), SV_len(empty), size_t, "%zu");
    return PASS;
}
