#include "parser.h"

#include "munit.h"

static MunitResult
p_simple_1(const MunitParameter params[], void* data) {
    munit_assert(1 == 1);

    return MUNIT_OK;
}

MunitTest parser_suite_tests[] = {
    {
        .name = "/simple_1",
        .test = p_simple_1,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
};