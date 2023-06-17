#include "token.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


char* token_to_string(const token_t* t) {
    const char* type_str  = token_type_to_str(t->kind);
    const char* template = "<type=%s, contents=\"%.*s\", position=(%d, %d)>\n";

    char* str = calloc(strlen(type_str) + strlen(template) + 8, sizeof(char));
    
    sprintf(str, template, type_str, t->contents_len, t->contents, t->debug_info.row, t->debug_info.col);
    return str;
}

void print_token(const token_t* t) {
    char* str = token_to_string(t);
    printf("%s", str);
    free(str);
    return;
}