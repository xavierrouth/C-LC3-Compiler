#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>
#include <stdbool.h>

#include "token.h"

#define MAX_NUM_PARSER_WARNINGS 4
#define MAX_NUM_PARSER_ERRORS 1 

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
    int16_t offset; // Offset from prev token.
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
    char* source;
    uint32_t source_size;
} parser_error_handler;

bool report_error(parser_error_t error);

void init_error_handler(const char* source, uint32_t source_size);

void print_errors();
void print_analysis_errors();

#endif
