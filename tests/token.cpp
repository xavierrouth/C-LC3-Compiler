#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "token.h"

using namespace std; 


TEST_CASE("test Tokens")
{
    const char* contents = "hell1o123123123123123123";
    token_dbg_t debug = {.col = 1, .row = 1};
    token_t t = {
        .kind = T_IDENTIFIER,
        .debug_info = {.col = 1, .row = 1},
        .contents = "does this work?",
        .contents_len = sizeof("does this work?") - 4
    };

    REQUIRE(1 == 1);
}
    