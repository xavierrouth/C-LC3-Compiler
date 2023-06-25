#include "parser.h"
#include "AST_visitor.h"
#include "munit.h"

static MunitResult
p_simple_1(const MunitParameter params[], void* data) {
    munit_assert(1 == 1);
    
    FILE* f;
    const char * path = "../tests/parser/simple-1.c";
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

    ast_node_enum nodes_gold[] = {
        A_PROGRAM,
        A_VAR_DECL,
        A_INTEGER_LITERAL,
        A_VAR_DECL,
        A_VAR_EXPR,
        A_VAR_DECL,
    };

    int gold_size = sizeof(nodes_gold) / sizeof(nodes_gold[0]);

    // Results and gold need to have the same size;
    ast_node_enum nodes_result[gold_size];

    int i = 0;

    init_lexer(file_buffer, fsize);
    init_parser(false);
    build_ast();
    ast_node_t* root = get_root();
    check_ast(root, nodes_result);
    free_ast(root);

    for (int i = 0; i < gold_size; i++) {
        munit_assert_int(nodes_result[i], ==, nodes_gold[i]);
    }

    free(file_buffer);

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