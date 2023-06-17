#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "token.h"
#include "AST.h"

typedef struct PARSER_STRUCT {
    token_t putback;
    // Symbol table
    // Type table
    ast_node_t* ast_root;
} parser_t;

void init_parser();

void build_ast();

ast_node_t* get_root();

void teardown_ast();

#endif