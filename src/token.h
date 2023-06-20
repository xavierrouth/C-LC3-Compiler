#ifndef TOKEN_H
#define TOKEN_H


#include <stddef.h>
#include <stdio.h>

typedef struct TOKEN_DEBUG_INFO_STRUCT{
    int row;
    int col;
} token_dbg_t;


typedef enum TOKEN_KIND_ENUM {
    T_BASE = 0, // 
    T_IDENTIFIER, 

    T_INTLITERAL,

    // ==== Operators ====
    T_ASSIGN, // = 

    T_EQUALITY, // ==
    T_ADD, // '+'
    T_SUB, // '-'
    T_MUL, // '*'
    T_DIV, // '/'

    T_BITAND, // '&'
    T_BITOR, // '|'

    T_LOGAND,
    T_LOGOR,

    T_XMARK,

    T_NOTEQUALS, // '!='

    // Binary Operators

    // Boolean Operators

    T_LT,
    T_GT,
    T_LT_EQ,
    T_GT_EQ,
    
    T_LPAREN, // '('
    T_RPAREN, // ')'
    T_LBRACE, // '{'
    T_RBRACE, // '}'
    T_LBRACKET, // '['
    T_RBRACKET, // ']'

    T_COLON, // ':'
    T_COMMA, // ','
    T_SEMICOLON, // ';'

    T_MOD, // '%'

    T_ARROW, // '->'

    T_ASSIGN_LSHIFT, // <<=

    // ==== Keywords ====
    // Types
    T_INT, // Only one supported
    T_VOID, 

    // Type declaration Describers
    T_CONST,

    // Conditionals
    T_GOTO,
    T_IF,
    T_ELSE,
    T_BREAK,
    T_SWITCH,
    T_CONTINUE,
    T_DEFAULT,
    T_DO,

    T_CASE,

    // Structs
    T_STRUCT,
    T_TYPEDEF,
    T_UNION,

    T_RETURN,
    T_FOR,
    T_SIZEOF,
    T_STATIC,
    T_WHILE,
    T_ENUM,
    T_EXTERN,
    T_CHAR,

    // Helper
    T_UKNOWN,
    T_INVALID,
    T_START,
    T_END
} token_enum;

static char buffer[4];

static const char * token_type_to_str(token_enum type) {
    switch(type) {
        case T_IDENTIFIER: return "T_IDENTIFIER";
        case T_INTLITERAL: return "T_INTLITERAL";
        case T_ASSIGN: return "T_ASSIGN";
        case T_ADD: return "T_ADD";
        case T_SUB: return "T_SUB";
        case T_MUL: return "T_MUL";
        case T_DIV: return "T_DIV";
        case T_BITAND: return "T_BITAND";
        case T_BITOR: return "T_BITOR";
        case T_LOGAND: return "T_LOGAND";
        case T_LOGOR: return "T_LOGOR";
        case T_XMARK: return "T_XMARK";
        case T_NOTEQUALS: return "T_NOTEQUALS";
        case T_LT: return "T_LT";
        case T_GT: return "T_GT";
        case T_LT_EQ: return "T_LT_EQ";
        case T_GT_EQ: return "T_GT_EQ";
        case T_LPAREN: return "T_LPAREN";
        case T_RPAREN: return "T_RPAREN";
        case T_LBRACE: return "T_LBRACE";
        case T_RBRACE: return "T_RBRACE";
        case T_LBRACKET: return "T_LBRACKET";
        case T_RBRACKET: return "T_RBRACKET";
        case T_COLON: return "T_COLON";
        case T_COMMA: return "T_COMMA";
        case T_SEMICOLON: return "T_SEMICOLON";
        case T_INT: return "T_INT";
        case T_RETURN: return "T_RETURN";
        case T_IF: return "T_IF";
        case T_EQUALITY: return "T_EQUALITY";
        case T_END: return "T_END";
        case T_VOID: return "T_VOID";
    }

    snprintf(buffer, 4, "%d", type);
    return buffer;
    //"Not printable";
}

typedef struct TOKEN_STRUCT 
{
    token_enum kind;
    token_dbg_t debug_info;
    const char* contents; 
    int contents_len;
} token_t;

void print_token(const token_t* token);

char* token_to_string(const token_t* token);

#endif