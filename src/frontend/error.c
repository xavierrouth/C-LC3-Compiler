#include <string.h>

#include "util/util.h"
#include "error.h"

parser_error_handler error_handler; // parser-scope error handler

static void print_error(parser_error_t error) {
    token_t token = error.invalid_token;

    // TODO: Should prevent random segfaults, can be handled better.
    if (token.contents == NULL)
        return;

    size_t token_length = strlen(token.contents);
    size_t len = strlen("Line # |") + error.offset;
    switch (error.type) {
        // Parser Errors:
        case ERROR_MISSING_SEMICOLON: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Expected semicolon.\n");
            print_line(token.row, error_handler.source, error_handler.source_size);
            printf_indent(token.col + token_length + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;
        }
        case ERROR_MISSING_EXPRESSION: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Expected an expression.\n");
            print_line(token.row, error_handler.source, error_handler.source_size);
            printf_indent(token.col + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;
        }
        case ERROR_UNEXPECTED_TOKEN: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Unexpected token.\n");
            print_line(token.row, error_handler.source, error_handler.source_size);
            printf_indent(token.col + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;
        }
        case ERROR_MISSING_RPAREN: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Expected closing parenthesis ')'.\n");
            print_line(token.row, error_handler.source, error_handler.source_size);
            printf_indent(token.col + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;
        }
        // Analysis Errors:
        case ERROR_SYMBOL_REDECLARED: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Redeclaration of '%s'.\n", token.contents);
            print_line(token.row, error_handler.source, error_handler.source_size);
            printf_indent(token.col + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;
        }
        case ERROR_SYMBOL_UNKNOWN: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Symbol '%s' was not declared in this scope.\n", token.contents);
            print_line(token.row, error_handler.source, error_handler.source_size);
            printf_indent(token.col + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;

        }
        default: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Something is wrong!\n");
            print_line(error.prev_token.row, error_handler.source, error_handler.source_size);
            printf_indent(token.col + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;
        }
    }
}

void print_errors() {
    static int i = 0; // Only prints each error once.
    for (; i < error_handler.num_errors; i++) {
        print_error(error_handler.errors[i]);
    }
    for (; i < error_handler.num_warnings; i++) {
        //print_error(error_handler.warnings[i]);
    }
}

void print_analysis_errors() {
    for (int i = 0; i < error_handler.num_errors; i++) {
        print_error(error_handler.errors[i]);
    }
}

void init_error_handler(const char* source, uint32_t source_size) {
    error_handler.source = source;
    error_handler.source_size = source_size;
}

bool report_error(parser_error_t error) {
    if (error_handler.num_errors == MAX_NUM_PARSER_ERRORS) {
        return false;
    }
    error_handler.errors[error_handler.num_errors++] = error;
    return true;
}
