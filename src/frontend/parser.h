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
 * Add proper error handling and error recovery.
*/

// Things we want to include are line / col information mostly.

typedef struct PARSER_STRUCT {
    token_t putback_stack[8];
    u8 putback_idx;
    ast_node_h ast_root;
} parser_t;

void init_parser(const char* source, u32 source_size);

void end_parse();
void build_ast();

ast_node_h get_root();

#endif
