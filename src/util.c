#include <stdio.h>
#include <stdint.h>

#include "util.h"


char print_buffer[128];

void printf_indent(int indent, char * string)
{
    printf("%*s%s", indent, "", string);
}

void print_line(int line, const char* file_buffer, int buffer_size) {
    // TODO: Error handling
    uint16_t curr_line = 1;
    uint16_t j = 0;
    for (uint16_t i = 0; i < buffer_size; i++) {
        // Check end of line_buffer
        if (curr_line == line) {
            // Find the line length
            for (j = i; j < buffer_size; j++) {
                if (file_buffer[j] == '\n') {
                    break;
                }
            }
            printf("Line %d | %.*s\n", line, (j-i), &file_buffer[i]);
            break;
        }
        if (file_buffer[i] == '\n') {
            curr_line++;
        }
    }
}

/**
void pretty_printf_indent_tree(char* indent_string, char branch_char, char* string) {
    printf("%s%c-%s", indent_string, branch_char, string);
}

void printf_indent_tree(int indent, char * string, char c) {
    
    if (indent > 0) {
        printf("%*s%c-%s", indent, "", c, string);   
    }
    
    printf("%*s%s", indent, "", string);
}
*/