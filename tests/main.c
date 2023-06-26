#include "munit.h"

#include "lexer_tests.h"
#include "parser_tests.h"

static MunitResult
test_compare(const MunitParameter params[], void* data) {
    munit_assert(1 == 1);
    return MUNIT_OK;
}

static MunitTest sanity[] = {
  {
        .name = "/sanity",
        .test = test_compare,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
};

extern MunitTest lexer_suite_tests[];
extern MunitTest parser_suite_tests[];

static const MunitSuite other_suites[] = { 
   { "/lexer", lexer_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE },
   { "/parser", parser_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE },  
   { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE } 
}; 

/* Now we'll actually declare the test suite.  You could do this in
 * the main function, or on the heap, or whatever you want. */
static const MunitSuite test_suite = {
  /* This string will be prepended to all test names in this suite;
   * for example, "/example/rand" will become "/µnit/example/rand".
   * Note that, while it doesn't really matter for the top-level
   * suite, NULL signal the end of an array of tests; you should use
   * an empty string ("") instead. */
  (char*) "",
  /* The first parameter is the array of test suites. */
  sanity,
  other_suites,
  1,
  /* Just like MUNIT_TEST_OPTION_NONE, you can provide
   * MUNIT_SUITE_OPTION_NONE or 0 to use the default settings. */
  MUNIT_SUITE_OPTION_NONE
};

/* This is only necessary for EXIT_SUCCESS and EXIT_FAILURE, which you
 * *should* be using but probably aren't (no, zero and non-zero don't
 * always mean success and failure).  I guess my point is that nothing
 * about µnit requires it. */
#include <stdlib.h>

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
  /* Finally, we'll actually run our test suite!  That second argument
   * is the user_data parameter which will be passed either to the
   * test or (if provided) the fixture setup function. */
  return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}
