#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "AST.h"
#include "codegen.h"
#include "analysis.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    //FILE* f = fopen("../src/test1.c", "rb");
    FILE* f;
    const char* path = argv[1];
    //const char * path = "../tests/lexer/simple-1.c";

    if ((f = fopen(path, "rb")) == NULL) { 
        printf("Invalid input file path.\n");
        return 1;
    }

    set_out_file("../out/out.asm");

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char* file_buffer = malloc(fsize + 1);
    fread(file_buffer, fsize, 1, f);
    fclose(f);

    file_buffer[fsize] = 0; //Set null terminator

    // Init Lexer
    
    init_lexer(file_buffer, fsize);
    token_t t = get_token();
    while(t.kind != T_END) {
        t = get_token();
        print_token(&t);
    }

    init_lexer(file_buffer, fsize);

    printf("Done Lexering\n");

    init_parser(true);
    printf("Done Init Parser\n");
    build_ast();

    printf("Done Building AST\n");

    ast_node_t* root = get_root();    

    /**
    for (int i = 0; i < 5; i++) {
        printf("%d\n", root->as.program.body.nodes[i]->as.literal.value);
    }
    */
    /**
    while ((root = root->data.stmt.next) != NULL) {
        printf("%d\n", root->data.value);
    }
    */
    /*
    print_ast_node(root, 0);
    print_ast_node(root->as.program.body.nodes[0], 1);
    print_ast_node(root->as.program.body.nodes[1], 1); 
    print_ast_node(root->as.program.body.nodes[1]->as.var_decl.initializer, 2); 
    print_ast_node(root->as.program.body.nodes[2], 1); 
    */

    print_ast(root);
    analysis(root);
    printf("Beginning Code gen:\n");
    emit_ast(root);
    free_ast(root);

    close_out_file();


    free(file_buffer);
    


    return 0;
}
