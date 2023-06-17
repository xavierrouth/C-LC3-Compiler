#ifndef LEXER_H
#define LEXER_H

#include "token.h"

typedef struct LEXER_STRUCT {
    const char * src;
    int index;
    int row;
    int col;
    char putback;
    long size;
} lexer_t;

void init_lexer(const char * src, long size);

token_t get_token(); 

#endif
