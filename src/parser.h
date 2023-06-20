#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "token.h"
#include "AST.h"

#include <stdbool.h>

/**
 * parser.h and parser.c implement the,
 * All parser state is stored as a static parser_t variable in the parser.c file.
 * Functions to build and traverse the ast will be provided in this interface.
 * 
 * TODO:
 * Implement much better / cleaner parsing, i.e 
 * add / modify expect_token() eat_token(), next_token(), putback_token()
 * as needed.
 * Add proper error handling and error recovery.
*/

typedef struct PARSER_STRUCT {
    token_t putback_stack[8];
    int putback_idx;
    bool error_mode; // Todo: make this into a struct with different error options. 
    // Symbol table
    // Type table
    ast_node_t* ast_root;
} parser_t;

void init_parser(bool error_mode);

void build_ast();

ast_node_t* get_root();

void teardown_ast();

#endif