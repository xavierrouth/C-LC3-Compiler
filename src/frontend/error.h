#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>
#include <stdbool.h>

#include "token.h"
#include "util/util.h"

#define MAX_NUM_PARSER_WARNINGS 4
#define MAX_NUM_PARSER_ERRORS 2 

typedef struct PARSER_ERROR_STRUCT {
    token_t invalid_token; // The token that was unexpected
    token_t prev_token; // The previous token in the token stream/
    enum {
        ERROR_MISSING_TOKEN,
        ERROR_MISSING_SEMICOLON,
        ERROR_MISSING_EXPRESSION,
        ERROR_MISSING_SOMETHING,
        ERROR_MISSING_RPAREN,
        ERROR_UNEXPECTED_TOKEN,
        ERROR_SYMBOL_REDECLARED,
        ERROR_SYMBOL_UNKNOWN,
        ERROR_GENERAL,
    } type;
    i8 offset; // Offset from prev token.
} parser_error_t;

typedef struct PARSER_ERROR_HANDLER {
    parser_error_t errors[MAX_NUM_PARSER_WARNINGS];
    parser_error_t warnings[MAX_NUM_PARSER_WARNINGS];
    u8 num_errors;
    u8 num_warnings;
    bool abort;
    bool print_errors;
    bool print_warnings;
    bool skip_statement;
    char* source;
    u32 source_size;
} parser_error_handler;

bool report_error(parser_error_t error);

void init_error_handler(const char* source, u32 source_size);

void print_errors();
void print_analysis_errors();


#endif
