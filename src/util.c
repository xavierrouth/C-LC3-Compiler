#include <stdio.h>
#include "util.h"

void printf_indent(int indent, char * string)
{
    printf("%*s%s", indent, "", string);
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