#include <string.h>

#include "util.h"
#include "error.h"

parser_error_handler error_handler; // parser-scope error handler

static void print_error(parser_error_t error) {
    token_t previous = error.prev_token;
    size_t token_length = strlen(previous.contents);
    size_t len = strlen("Line # |");
    switch (error.type) {
        
        case ERROR_MISSING_SEMICOLON: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Expected semicolon.\n");
            print_line(previous.debug_info.row, error_handler.source, error_handler.source_size);
            printf_indent(previous.debug_info.col + token_length + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;
        }
        case ERROR_MISSING_EXPRESSION: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Expected an expression.\n");
            print_line(error.invalid_token.debug_info.row, error_handler.source, error_handler.source_size);
            printf_indent(error.invalid_token.debug_info.col + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;
        }
        case ERROR_UNEXPECTED_TOKEN: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Unexpected token.\n");
            print_line(error.invalid_token.debug_info.row, error_handler.source, error_handler.source_size);
            printf_indent(error.invalid_token.debug_info.col + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;
        }
        case ERROR_SYMBOL_REDECLARED: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Redeclaration of '%s'.\n", error.invalid_token.contents);
            print_line(error.invalid_token.debug_info.row, error_handler.source, error_handler.source_size);
            printf_indent(error.invalid_token.debug_info.col + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;
        }
        case ERROR_SYMBOL_UNKNOWN: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Symbol '%s' was not declared in this scope.\n", error.invalid_token.contents);
            print_line(error.invalid_token.debug_info.row, error_handler.source, error_handler.source_size);
            printf_indent(error.invalid_token.debug_info.col + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;

        }
        default: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Something is wrong!\n");
            print_line(error.prev_token.debug_info.row, error_handler.source, error_handler.source_size);
            printf_indent(previous.debug_info.col + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
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

void report_error(parser_error_t error) {
    if (error_handler.num_errors == MAX_NUM_PARSER_ERRORS) {
        end_parse();
    }
    error_handler.errors[error_handler.num_errors++] = error;
}
    