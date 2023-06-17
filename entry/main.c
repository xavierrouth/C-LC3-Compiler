#include "token.h"
#include "lexer.h"
#include "parser.h"

#include <stdlib.h>
#include <stdio.h>



int main(int argc, char **argv) {
    //FILE* f = fopen("../src/test1.c", "rb");
    FILE* f = fopen("../test_src/test1.c", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char* file_buffer = malloc(fsize + 1);
    fread(file_buffer, fsize, 1, f);
    fclose(f);

    file_buffer[fsize] = 0; //Set null terminator

    const char* contents = "hell1o123123123123123123";
    token_dbg_t debug = {.col = 1, .row = 1};
    token_t t = {
        .kind = T_IDENTIFIER,
        .debug_info = {.col = 1, .row = 1},
        .contents = "does this work?",
        .contents_len = sizeof("does this work?") - 4
    };

    print_token(&t);
    // Init Lexer
    
    init_lexer(file_buffer, fsize);
    while(t.kind != T_END) {
        t = get_token();
        print_token(&t);
    }

    printf("Done Lexering\n");

    init_parser();
    printf("Done Init Parser\n");
    build_ast();

    printf("Done Building AST\n");

    ast_node_t* root = get_root();
    ast_node_t* c = root->program.cmpd;

    

    for (int i = 0; i < 5; i++) {
        printf("%d\n", c->cmpd_stmt.statements[i]->lit_value);
    }
    /**
    while ((root = root->data.stmt.next) != NULL) {
        printf("%d\n", root->data.value);
    }
    */
    


    


    free(file_buffer);
    


    return 0;
}