#include "str_view.h"
#include "test.h"

#include <stdbool.h>

int
main(void) {
    CHECK(SV_is_empty(SV_from("")), true, bool, "%d");
    CHECK(*SV_null(), '\0', char, "%c");
    return PASS;
}
