#include "lexer.h"
#include "util.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>

static lexer_t Lexer;

char id_buffer[2048];
int id_buffer_idx;

void init_lexer(const char * src, long size) {
    Lexer.src = src;
    Lexer.index = 0;
    Lexer.row = 1;
    Lexer.col = 1;
    Lexer.putback = 0;
    Lexer.size = size;
    id_buffer_idx = 0;
    return;
}

static int chrpos(char *s, int c) {
	char *p;

	p = strchr(s, c);
	return p? p-s: -1;
}

static char next() {
    char c;

    // One character buffer.
    // I think this is better than looking one character ahead.
    if (Lexer.putback) {
        c = Lexer.putback;
        Lexer.putback = 0;
        return c;
    }

    if (Lexer.index > Lexer.size) {
        return EOF;
    }

    c = Lexer.src[Lexer.index++];
    Lexer.col++;
    if (c == '\n') {
        Lexer.row++;
        Lexer.col = 0;
    }    

    return c;
}

void putback(char c) {
    Lexer.putback = c;
}

static int is_whitespace(char c) {
    if (' ' == c || '\t' == c || '\n' == c || '\r' == c || '\f' == c)
        return 1;
    else 
        return 0;
}

static char skip() {
    char c = next();
    while (is_whitespace(c)) {
        c = next();
    }
    return c;
}

static void skip_block_comment() {
    char c;
    while (c = next()) {
        if (c == EOF) {
            // Error and return
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Expected */ to end block comment.\n");
            return;
        }
        if (c == '*') {
            if (next() == '/') {
                return;
            }
        }
        continue;
    }
}

static void next_line() {
    char c = next();
    while (!(c == '\n' || c == '\r')) {
        c = next();
    }
}

static uint16_t scan_symbol(char c) {
    uint16_t length = 0;
    while (isalpha(c) || isdigit(c) || '_' == c) {
        c = next();
        length++;
    }
    putback(c);
    return length;
}

static uint16_t scan_int(char c) {
    uint16_t length = 0;
    static int float_error = 0;
    while (isdigit(c)) {
        c = next();
        length++;
    }
    if (c == '.') {
        // Lexer Error:
        if (!float_error) {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Floats not supported on LC3 architecture, rounding down.\n");
            float_error = 1;
        }
        
        c = next();
        while (isdigit(c)) {
            c = next();
        }
    }
    
    putback(c);
    return length;
}

static uint16_t scan_string() {
    uint16_t length = 0;
    char c;
    while (c = next()) {
        length++;
        if (c == '\\' && next() == '"') {
            length++;
            continue;
        }
        
        if (c == '"') {
            break;
        }
    }
    return length;
}

static token_enum keyword(char *s) {
	switch (*s) {
    /**
	case '#':
		switch (s[1]) {
		case 'd':
			if (!strcmp(s, "#define")) return P_DEFINE;
			break;
		case 'e':
			if (!strcmp(s, "#else")) return P_ELSE;
			if (!strcmp(s, "#endif")) return P_ENDIF;
			break;
		case 'i':
			if (!strcmp(s, "#ifdef")) return P_IFDEF;
			if (!strcmp(s, "#ifndef")) return P_IFNDEF;
			if (!strcmp(s, "#include")) return P_INCLUDE;
			break;
		case 'u':
			if (!strcmp(s, "#undef")) return P_UNDEF;
			break;
		}
		break;
    */
    case 'a':
        if (!strcmp(s, "auto")) return T_AUTO;
		break;
	case 'b':
		if (!strcmp(s, "break")) return T_BREAK;
		break;
	case 'c':
		if (!strcmp(s, "case")) return T_CASE;
		if (!strcmp(s, "char")) return T_CHAR;
		if (!strcmp(s, "continue")) return T_CONTINUE;
		break;
	case 'd':
		if (!strcmp(s, "default")) return T_DEFAULT;
		if (!strcmp(s, "do")) return T_DO;
		break;
	case 'e':
		if (!strcmp(s, "else")) return T_ELSE;
		if (!strcmp(s, "enum")) return T_ENUM;
		if (!strcmp(s, "extern")) return T_EXTERN;
		break;
	case 'f':
		if (!strcmp(s, "for")) return T_FOR;
		break;
	case 'i':
		if (!strcmp(s, "if")) return T_IF;
		if (!strcmp(s, "int")) return T_INT;
		break;
	case 'r':
		if (!strcmp(s, "return")) return T_RETURN;
		break;
	case 's':
		if (!strcmp(s, "sizeof")) return T_SIZEOF;
		if (!strcmp(s, "static")) return T_STATIC;
		if (!strcmp(s, "switch")) return T_SWITCH;
		break;
    case 't':
        if (!strcmp(s, "typedef")) return T_TYPEDEF;
        break;
	case 'v':
		if (!strcmp(s, "void")) return T_VOID;
		break;
	case 'w':
		if (!strcmp(s, "while")) return T_WHILE;
		break;
    case '_':
        if (!strcmp(s, "__asm")) return T_ASM;
        break;
    }
	return 0;
}

static token_enum char_to_token_type(char c) {
    switch (c) {
        case '{': return T_LBRACE;
        case '}': return T_RBRACE;
        case '[': return T_LBRACKET;
        case ']': return T_RBRACKET;
        case '(': return T_LPAREN;
        case ')': return T_RPAREN;
        case '+': return T_ADD;
        case '-': return T_SUB;
        case ';': return T_SEMICOLON;
        case ',': return T_COMMA;
        case '~': return T_BITFLIP;
        case '?': return T_TERNARY;
        case ':': return T_COLON;
        case '.': return T_DOT;
    }
    return T_INVALID;
}

static void move_to_str_buffer(char* contents, int len) {
    // Check errors::
    strncpy(id_buffer + id_buffer_idx, contents, len);
    id_buffer_idx += len + 1; // Leave space for null ptr.
    return;
}
// Get the next token, you probably don't want to be calling this from the parser.
token_t get_token() {
    token_t token;
    while (1 == 1) {
        char c = skip();
        // Token is starting
        char* contents = Lexer.src + Lexer.index - 1; //Maybe -1;
        token.contents = &id_buffer[id_buffer_idx];
        //&(id_buffer[id_buffer_idx]);
        token.debug_info.col = Lexer.col;
        token.debug_info.row = Lexer.row;
        switch(c) {
            case '\0':
                token.kind = T_END;
                return token;
            case '!':
                if ((c = next()) == '=') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_NOTEQUALS;
                    return token;
                }
                else {
                    putback(c);
                    move_to_str_buffer(contents, 1);
                    token.kind = T_LOGNOT;
                    return token;
                }
            case '&':
                if ((c = next()) == '&') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_LOGAND;
                    return token;
                }
                else if (c == '=') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_ASSIGN_BITAND;
                    return token;
                }
                else {
                    putback(c);
                    move_to_str_buffer(contents, 1);
                    token.kind = T_BITAND;
                    return token;
                }
            case '|':
                if ((c = next()) == '|') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_LOGOR;
                    return token;
                }
                else if (c == '=') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_ASSIGN_BITOR;
                    return token;
                }
                else {
                    putback(c);
                    move_to_str_buffer(contents, 1);
                    token.kind = T_BITOR;
                    return token;
                }
            
            case '=':
                if ((c = next()) == '=') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_EQUALS;
                    return token;
                }
                else {
                    putback(c);
                    move_to_str_buffer(contents, 1);
                    token.kind = T_ASSIGN;
                    return token;
                }
            case '%':
                if ((c = next()) == '=') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_ASSIGN_MOD;
                    return token;
                }
                else {
                    putback(c);
                    move_to_str_buffer(contents, 1);
                    token.kind = T_MOD;
                    return token;
                }
            case '/':
                if ((c = next()) == '=') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_ASSIGN_DIV;
                    return token;
                } 
                else if (c == '/') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_COMMENT;
                    next_line();
                    return get_token();
                }
                else if (c == '*') {
                    skip_block_comment();
                    return get_token();
                }
                else {
                    putback(c);
                    move_to_str_buffer(contents, 1);
                    token.kind = T_DIV;
                    return token;
                }
                
            case '-':
                if ((c = next()) == '-') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_DECREMENT;
                    return token;
                }
                // Don't need to do c = next, as it is already next from evaluating above case.
                else if (c == '>') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_ARROW;
                    return token;
                }
                else if (c == '=') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_ASSIGN_SUB;
                    return token;
                }
                else {
                    putback(c);
                    move_to_str_buffer(contents, 1);
                    token.kind = T_SUB;
                    return token;
                }
            case '+':
                if((c = next()) == '=') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_ASSIGN_ADD;
                    return token;
                }
                else if (c == '+') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_INCREMENT;
                    return token;
                }
                else {
                    putback(c);
                    move_to_str_buffer(contents, 1);
                    token.kind = T_ADD;
                    return token;
                }
            case '*':
                if ((c = next()) == '=') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_ASSIGN_MUL;
                    return token;
                }
                else {
                    putback(c);
                    move_to_str_buffer(contents, 1);
                    token.kind = T_MUL;
                    return token;
                }
            case '<':
                if ((c = next()) == '=') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_LT_EQUAL;
                    return token;
                }
                else if (c == '<') {
                    if ((c = next()) == '=') {
                        move_to_str_buffer(contents, 3);
                        token.kind = T_ASSIGN_LSHIFT;
                    }
                    else {
                        putback(c);
                        move_to_str_buffer(contents, 2);
                        token.kind = T_LEFTSHIFT;
                    }
                    return token;
                }
                else {
                    putback(c);
                    move_to_str_buffer(contents, 1);
                    token.kind = T_LT;
                    return token;
                }
            case '>':
                if ((c = next()) == '=') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_GT_EQUAL;
                    return token;
                }
                else if (c == '>') {
                    if ((c = next()) == '=') {
                        move_to_str_buffer(contents, 3);
                        token.kind = T_ASSIGN_RSHIFT;
                    }
                    else {
                        putback(c);
                        move_to_str_buffer(contents, 2);
                        token.kind = T_RIGHTSHIFT;
                    }
                    return token;
                }
                else {
                    putback(c);
                    move_to_str_buffer(contents, 1);
                    token.kind = T_GT;
                    return token;
                }
            case '^':
                if ((c = next()) == '=') {
                    move_to_str_buffer(contents, 2);
                    token.kind = T_ASSIGN_BITXOR;
                    return token;
                }
                else {
                    putback(c);
                    move_to_str_buffer(contents, 1);
                    token.kind = T_BITXOR;
                    return token;
                }
            case '"': {
                uint16_t len = scan_string() - 1;
                contents++;
                move_to_str_buffer(contents, len);
                token.kind = T_STRLITERAL;
                return token;
            }
            // Single token ones:
            case '~':
            case '(':
            case ')':
            case ';':
            case ',':
            case '{':
            case '}':
            case ':':
            case '[':
            case ']':
            case '?':
            case '.':
                token.kind = char_to_token_type(c);
                move_to_str_buffer(contents, 1);
                return token;
            default:
                if (isdigit(c)) {
                    move_to_str_buffer(contents, scan_int(c));
                    token.kind = T_INTLITERAL;
                    return token;
                }
                else if (isalpha(c) || '_' == c) {
                    move_to_str_buffer(contents, scan_symbol(c));
                    if ((token.kind = keyword(token.contents)) != 0) {
                        return token;
                    }
                    else {
                        token.kind = T_IDENTIFIER;
                        return token;
                    }
					    
                }
                token.kind = T_END;
                return token;
        }
    }
    
    printf("Should not have gotten here.\n");
    return token;
}

