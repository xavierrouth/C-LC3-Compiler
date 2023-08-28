#include "token.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_TOKEN_DEBUG_SIZE 128

char str_buffer[MAX_TOKEN_DEBUG_SIZE];

char* token_to_debug(const token_t* t) {
    const char* as_string  = token_to_string(t);
    const char* template = "<type=%s, contents=\"%s\", position=(%d, %d)>\n";
    
    sprintf(str_buffer, template, as_string, t->contents, t->row, t->col);
    return str_buffer;
}

void print_token(const token_t* t) {
    char* str = token_to_string(t);
    printf("%s", str);
    return;
}

#define TOKEN_STRING(name, string, ...) case name: return string;

char* token_to_string(const token_t* t) {
    switch (t->kind) {
        FOR_SIMPLE_TOKENS(TOKEN_STRING)
        case T_IDENTIFIER: return t->contents;
        case T_STRLITERAL: return t->contents;
        case T_ASM: return t->contents;
        case T_INTLITERAL: return t->contents;
        default: printf("Uh OH!\n"); return "Uh OH!";
    }
}

