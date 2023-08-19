#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "util.h"
#include "memory/bump_allocator.h"


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

#define STRING_BUFFER_SIZE 2048

char string_buffer[STRING_BUFFER_SIZE];

static bump_allocator_t string_allocator = {
    .start = &string_buffer[0],
    .end = &string_buffer[STRING_BUFFER_SIZE - 1],
    .ptr = &string_buffer[STRING_BUFFER_SIZE - 1]
};

char* format(char* fmt, ...) {
    
    va_list args_const;
    va_start(args_const, fmt);

    va_list args_use;
    va_copy(args_use, args_const);

    /**
    vprintf(fmt, args_use);
    va_copy(args_use, args_const);
    */

    uint32_t length = vsnprintf(NULL, 0, fmt, args_use) + 1;
    va_copy(args_use, args_const);
    char* buff = bump_allocate(&string_allocator, sizeof(char), sizeof(char) * length);    
    vsnprintf(buff, length, fmt, args_use);

    //printf(buff);

    va_end(args_use);
    va_end(args_const);
    return buff;
}

int align_up(int n, int align) {
    return (n + align - 1) / align * align;
}
