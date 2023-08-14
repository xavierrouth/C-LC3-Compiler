#include "token.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char str_buffer[128];

char* token_to_string(const token_t* t) {
    const char* type_str  = token_type_to_str(t->kind);
    const char* template = "<type=%s, contents=\"%s\", position=(%d, %d)>\n";

    // Max ID size is 64??
    //char* str = calloc(strlen(type_str) + strlen(template) + strlen(t->contents), sizeof(char));
    
    sprintf(str_buffer, template, type_str, t->contents, t->row, t->col);
    return str_buffer;
}

void print_token(const token_t* t) {
    char* str = token_to_string(t);
    printf("%s", str);
    free(str);
    return;
}
