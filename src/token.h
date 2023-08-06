#ifndef TOKEN_H
#define TOKEN_H


#include <stddef.h>
#include <stdio.h>

typedef struct TOKEN_DEBUG_INFO_STRUCT{
    int row;
    int col;
} token_dbg_t;


// Rough groupings, some of the tokens take different meanings depending on the context.
typedef enum TOKEN_KIND_ENUM {
    T_INVALID = -1, // 
    // ==== Operators ====
    T_ASSIGN, // = 

    T_ADD, // '+'
    T_SUB, // '-'
    T_MUL, // '*'
    T_DIV, // '/'
    T_MOD, // '%'

    T_LOGAND, // '&&'
    T_LOGOR, // '||'
    T_LOGNOT, // '!'

    T_INCREMENT, // '++'
    T_DECREMENT, // '--'

    // Binary Operators
    T_BITAND, // '&'
    T_BITOR, // '|'
    T_BITXOR, // '^'
    T_BITFLIP, // '~'
    T_LEFTSHIFT, // '<<'
    T_RIGHTSHIFT, // '>>'

    // Boolean Operators
    T_LT, // '<'
    T_GT, // '>'
    T_LT_EQUAL, // '<='
    T_GT_EQUAL, // '>='
    T_NOTEQUALS, // '!='
    T_EQUALS, // '==' 
    
    T_DOT, // '.'
    T_ARROW, // '->'
    T_TERNARY, // '?'

    // Assignment operators
    T_ASSIGN_LSHIFT, // <<=
    T_ASSIGN_RSHIFT, // >>=
    T_ASSIGN_MUL, // *=
    T_ASSIGN_ADD, // '+='
    T_ASSIGN_SUB, // '-='
    T_ASSIGN_DIV, // '/='
    T_ASSIGN_MOD, // '%='
    T_ASSIGN_BITAND, // '&='
    T_ASSIGN_BITXOR, // '^='
    T_ASSIGN_BITOR, // '|='

    // Punctuation
    T_LPAREN, // '('
    T_RPAREN, // ')'
    T_LBRACKET, // '['
    T_RBRACKET, // ']'

    T_COLON, // ':'
    T_COMMA, // ','
    T_SEMICOLON, // ';'

    T_LBRACE, // '{'
    T_RBRACE, // '}'

    // Symbols / Literals
    T_IDENTIFIER, 
    T_INTLITERAL,
    T_COMMENT,

    // ==== Keywords ====
    T_SIZEOF,

    // Types
    T_INT, 
    T_VOID, 
    T_CHAR,

    // Type declaration Describers
    T_CONST,
    T_VOLTAILE,
    T_EXTERN,
    T_STATIC,
    T_AUTO,
    T_REGISTER,
    T_TYPEDEF,
    T_UNSIGNED,
    T_SIGNED,
    T_FLOAT,
    T_DOUBLE,
    T_LONG,
    T_SHORT,    

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

    // Loops:
    T_WHILE,
    T_FOR,
    
    // Structs
    
    T_STRUCT,
    T_UNION,
    T_ENUM,

    // Return
    T_RETURN,
    
    // Helper
    T_START,
    T_END
} token_enum;

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
        case T_LOGNOT: return "T_LOGNOT";
        case T_NOTEQUALS: return "T_NOTEQUALS";
        case T_LT: return "T_LT";
        case T_GT: return "T_GT";
        case T_LT_EQUAL: return "T_LT_EQUAL";
        case T_GT_EQUAL: return "T_GT_EQUAL";
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
        case T_EQUALS: return "T_EQUALS";
        case T_END: return "T_END";
        case T_VOID: return "T_VOID";
        case T_DECREMENT: return "T_DECREMENT";
        case T_INCREMENT: return "T_INCREMENT";
        case T_BITXOR: return "T_BITXOR";
        case T_ARROW: return "T_ARROW";
        case T_TERNARY: return "T_TERNARY";
        default:
            break;
    }

    return "token_type_to_str failed";
    //"Not printable";
}

typedef struct TOKEN_STRUCT 
{
    token_enum kind;
    token_dbg_t debug_info;
    char* contents; 
} token_t;

void print_token(const token_t* token);

char* token_to_string(const token_t* token);

#endif
