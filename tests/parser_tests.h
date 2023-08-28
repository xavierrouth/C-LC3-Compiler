#include "frontend/parser.h"
#include "frontend/AST.h"
#include "munit.h"

#include <stdbool.h>

static MunitResult test_helper(const ast_node_enum* nodes_gold, int gold_size, const char* path, bool debug) {
    munit_assert(1 == 1);
    
    FILE* f;
    
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

    // Results and gold need to have the same size;
    ast_node_enum nodes_result[gold_size];

    int i = 0;

    init_lexer(file_buffer, fsize);
    init_parser(file_buffer, fsize);
    build_ast();
    ast_node_h root = get_root();
    if (debug) {
        printf("\n");
        print_ast(root);
    }
        
    check_ast(root, nodes_result);

    for (int i = 0; i < gold_size; i++) {
        munit_assert_int(nodes_result[i], ==, nodes_gold[i]);
        if (debug) {
            print_ast_node(nodes_gold[i], 1);
        }
    }

    free(file_buffer);

    return MUNIT_OK;
}

static MunitResult
p_simple_1(const MunitParameter params[], void* data) {
    ast_node_enum nodes_gold[] = {
        A_PROGRAM,
        A_VAR_DECL,
        A_INTEGER_LITERAL,
        A_VAR_DECL,
        A_SYMBOL_REF,
        A_VAR_DECL,
    };
    int gold_size = sizeof(nodes_gold) / sizeof(nodes_gold[0]);
    //printf("size:%d\n", gold_size);
    const char * path = "../tests/parser/simple-1.c";
    return test_helper(nodes_gold, gold_size, path, false);
}

static MunitResult
p_simple_2(const MunitParameter params[], void* data) {
    ast_node_enum nodes_gold[] = {
        A_PROGRAM,
        A_FUNCTION_DECL,
        A_COMPOUND_STMT,
        A_VAR_DECL,
        A_VAR_DECL,
        A_VAR_DECL,
        A_RETURN_STMT,
        A_INTEGER_LITERAL
    };
    int gold_size = sizeof(nodes_gold) / sizeof(nodes_gold[0]);
    //printf("size:%d\n", gold_size);
    const char * path = "../tests/parser/simple-2.c";
    return test_helper(nodes_gold, gold_size, path, false);
}

static MunitResult
p_func_decl_1(const MunitParameter params[], void* data) {
    ast_node_enum nodes_gold[] = {
        A_PROGRAM,
        A_FUNCTION_DECL,
        A_COMPOUND_STMT,
        A_RETURN_STMT,
        A_BINARY_EXPR,
        A_BINARY_EXPR,
        A_INTEGER_LITERAL,
        
        A_FUNCTION_CALL,
        A_SYMBOL_REF,
        A_INTEGER_LITERAL,
        A_INTEGER_LITERAL,
        A_INTEGER_LITERAL
    };
    int gold_size = sizeof(nodes_gold) / sizeof(nodes_gold[0]);
    //printf("size:%d\n", gold_size);
    const char * path = "../tests/parser/func-decl-1.c";
    return test_helper(nodes_gold, gold_size, path, false);
}

static MunitResult
p_func_decl_2(const MunitParameter params[], void* data) {
    ast_node_enum nodes_gold[] = {
        A_PROGRAM,
        A_FUNCTION_DECL, // test
        A_PARAM_DECL,
        A_PARAM_DECL,
        A_COMPOUND_STMT,
        A_RETURN_STMT,
        A_BINARY_EXPR,
        A_BINARY_EXPR,
        A_SYMBOL_REF,
        A_SYMBOL_REF,
        A_INTEGER_LITERAL,
        
        A_FUNCTION_DECL,
        A_COMPOUND_STMT,
        A_RETURN_STMT,
        A_BINARY_EXPR,
        A_BINARY_EXPR,
        A_INTEGER_LITERAL,
        
        A_FUNCTION_CALL,
        A_SYMBOL_REF,
        A_INTEGER_LITERAL,
        A_INTEGER_LITERAL,
        A_INTEGER_LITERAL
    };
    int gold_size = sizeof(nodes_gold) / sizeof(nodes_gold[0]);
    //printf("size:%d\n", gold_size);
    const char * path = "../tests/parser/func-decl-2.c";
    return test_helper(nodes_gold, gold_size, path, false);
}

static MunitResult
p_if_stmt_1(const MunitParameter params[], void* data) {
    ast_node_enum nodes_gold[] = {
        A_PROGRAM,
        A_FUNCTION_DECL,
        A_COMPOUND_STMT,
        A_VAR_DECL,
        A_INTEGER_LITERAL,
        A_IF_STMT,
        A_BINARY_EXPR,
        A_SYMBOL_REF,
        A_INTEGER_LITERAL,
        A_COMPOUND_STMT,
        A_VAR_DECL,
        A_RETURN_STMT,
        A_INTEGER_LITERAL
    };
    int gold_size = sizeof(nodes_gold) / sizeof(nodes_gold[0]);
    //printf("size:%d\n", gold_size);
    const char * path = "../tests/parser/if-stmt-1.c";
    return test_helper(nodes_gold, gold_size, path, false);
}

static MunitResult
p_if_stmt_2(const MunitParameter params[], void* data) {
    ast_node_enum nodes_gold[] = {
        A_PROGRAM,
        A_FUNCTION_DECL, // test
        A_PARAM_DECL,
        A_PARAM_DECL,
        A_COMPOUND_STMT,
        A_RETURN_STMT,
        A_BINARY_EXPR,
    };
    int gold_size = sizeof(nodes_gold) / sizeof(nodes_gold[0]);
    //printf("size:%d\n", gold_size);
    const char * path = "../tests/parser/if-stmt-2.c";
    return test_helper(nodes_gold, gold_size, path, false);
}

static MunitResult
p_if_stmt_3(const MunitParameter params[], void* data) {
    ast_node_enum nodes_gold[] = {
        A_PROGRAM,
        A_FUNCTION_DECL,
        A_COMPOUND_STMT,
        A_VAR_DECL,
        A_INTEGER_LITERAL,
        
        A_IF_STMT,
        A_BINARY_EXPR,
        A_SYMBOL_REF,
        A_INTEGER_LITERAL,
        A_COMPOUND_STMT,
        A_IF_STMT,
        A_BINARY_EXPR,
        A_INTEGER_LITERAL,
        A_INTEGER_LITERAL,
        A_COMPOUND_STMT,
        A_RETURN_STMT,
        A_BINARY_EXPR,
        A_INTEGER_LITERAL,
        A_INTEGER_LITERAL
    };
    int gold_size = sizeof(nodes_gold) / sizeof(nodes_gold[0]);
    //printf("size:%d\n", gold_size);
    const char * path = "../tests/parser/if-stmt-3.c";
    return test_helper(nodes_gold, gold_size, path, false);
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
    {
        .name = "/simple_2",
        .test = p_simple_2,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },
    {
        .name = "/func_decl_1",
        .test = p_func_decl_1,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },{
        .name = "/func_decl_2",
        .test = p_func_decl_2,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },{
        .name = "/if_stmt_1",
        .test = p_if_stmt_1,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },{
        .name = "/if_stmt_2",
        .test = p_if_stmt_2,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },{
        .name = "/if_stmt_3",
        .test = p_if_stmt_3,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
};
