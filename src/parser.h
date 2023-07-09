#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "token.h"
#include "AST.h"

#include <stdbool.h>
#define MAX_NUM_PARSER_WARNINGS 4
#define MAX_NUM_PARSER_ERRORS 1 

/**
 * parser.h and parser.c implement the,
 * All parser state is stored as a static parser_t variable in the parser.c file.
 * Functions to build and traverse the ast will be provided in this interface.
 * 
 * TODO:
 * Add proper error handling and error recovery.
*/

// Things we want to include are line / col information mostly.
typedef struct PARSER_ERROR_STRUCT {
    token_t invalid_token; // The token that was unexpected
    token_t prev_token; // The previous token in the token stream/
    enum {
        ERROR_MISSING_TOKEN,
        ERROR_MISSING_SEMICOLON,
        ERROR_MISSING_EXPRESSION,
        ERROR_MISSING_SOMETHING,
        ERROR_GENERAL,
    } type;
} parser_error_t;

typedef struct PARSER_ERROR_HANDLER {
    parser_error_t errors[8];
    parser_error_t warnings[8];
    parser_error_t in_construction; // We are trying to find out more information abotu this error!
    int16_t num_errors;
    int16_t num_warnings;
    bool abort;
    bool print_errors;
    bool print_warnings;
    bool skip_statement;
} parser_error_handler;

typedef struct PARSER_STRUCT {
    token_t putback_stack[8];
    uint32_t putback_idx;
    ast_node_t ast_root;
    const char* source; // Used for error handling.
    uint32_t source_size;
} parser_t;

void init_parser(const char* source, uint32_t source_size);

void build_ast();

ast_node_t get_root();

#endif
