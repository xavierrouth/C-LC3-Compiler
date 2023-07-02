#ifndef UTIL_H
#define UTIL_H

extern char print_buffer[128];

void printf_indent(int indent, char * string);

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
