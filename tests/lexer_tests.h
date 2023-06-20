#include "lexer.h"
#include "token.h"

#include "munit.h"

static MunitResult
l_simple_1(const MunitParameter params[], void* data) {
    munit_assert(1 == 1);
    
    FILE* f;
    const char * path = "../tests/lexer/simple-1.c";
    if ((f = fopen(path, "rb")) == NULL) { 
        printf("Invalid input file path.\n");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char* file_buffer = (char*) malloc(fsize + 1);
    fread(file_buffer, fsize, 1, f);
    fclose(f);

    file_buffer[fsize] = 0; //Set null terminator

    token_enum token_gold[] = {
        T_LPAREN,
        T_RPAREN,
        T_INT,
        T_IDENTIFIER,
        T_LPAREN,
        T_RPAREN,
        T_LBRACE,
        T_IF,
        T_LPAREN,
        T_IDENTIFIER,
        T_EQUALITY,
        T_INTLITERAL,
        T_RPAREN,
        T_IDENTIFIER,
        T_LPAREN,
        T_RPAREN,
        T_SEMICOLON,
        T_RETURN,
        T_IDENTIFIER,
        T_SEMICOLON,
        T_RBRACE,
    };

    int i = 0;

    init_lexer(file_buffer, fsize);
    token_t t = get_token();
    while(t.kind != T_END) {
        
        munit_assert_int(t.kind, ==, token_gold[i++]);
        t = get_token();
        //print_token(&t);
    }

    return MUNIT_OK;
}

static MunitResult
l_simple_2(const MunitParameter params[], void* data) {
    munit_assert(1 == 1);
    
    FILE* f;
    const char * path = "../tests/lexer/simple-2.c";
    if ((f = fopen(path, "rb")) == NULL) { 
        printf("Invalid input file path.\n");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char* file_buffer = (char*) malloc(fsize + 1);
    fread(file_buffer, fsize, 1, f);
    fclose(f);

    file_buffer[fsize] = 0; //Set null terminator

    token_enum token_gold[] = {
        T_INT,
        T_IDENTIFIER,
        T_SEMICOLON,
        T_INT,
        T_IDENTIFIER,
        T_ASSIGN,
        T_INTLITERAL,
        T_SEMICOLON,
        T_VOID,
        T_IDENTIFIER,
        T_SEMICOLON
    };

    int i = 0;

    init_lexer(file_buffer, fsize);
    token_t t = get_token();
    while(t.kind != T_END) {
        
        munit_assert_int(t.kind, ==, token_gold[i++]);
        t = get_token();
        //print_token(&t);
    }

    return MUNIT_OK;
}

MunitTest lexer_suite_tests[] = {
    {
        .name = "/simple_1",
        .test = l_simple_1,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },
    {
        .name = "/simple_2",
        .test = l_simple_2,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
};