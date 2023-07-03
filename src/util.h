#ifndef UTIL_H
#define UTIL_H

extern char print_buffer[128];

void printf_indent(int indent, char * string);

void print_line(int line, const char* file_buffer, int buffer_size);

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define CAT(a, b) a##b
#define PASTE(a, b) CAT(a, b)
#define JOIN(prefix, name) PASTE(prefix, PASTE(_, name))
#define _JOIN(prefix, name) PASTE(_, PASTE(prefix, PASTE(_, name)))

#define SWAP(TYPE, a, b)                                                                                               \
    {                                                                                                                  \
        TYPE temp = *(a);                                                                                              \
        *(a) = *(b);                                                                                                   \
        *(b) = temp;                                                                                                   \
    }

#define LEN(a) (sizeof(a) / sizeof(*(a)))


#endif 
