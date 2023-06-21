#include "lexer.h"
#include <string.h>
#include <ctype.h>

static lexer_t Lexer;

void init_lexer(const char * src, long size) {
    Lexer.src = src;
    Lexer.index = 0;
    Lexer.row = 1;
    Lexer.col = 1;
    Lexer.putback = 0;
    Lexer.size = size;
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
        return '\0';
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

static int scan_symbol(char c) {
    int length = 0;
    while (isalpha(c) || isdigit(c) || '_' == c) {
        c = next();
        length++;
    }
    putback(c);
    return length;
}

static int scan_int(char c) {
    int length = 0;
    while (isdigit(c)) {
        c = next();
        length++;
    }
    putback(c);
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
	case 'v':
		if (!strcmp(s, "void")) return T_VOID;
		break;
	case 'w':
		if (!strcmp(s, "while")) return T_WHILE;
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
    }
    return T_DEFAULT;
}

// Get the next token
token_t get_token() {
    token_t token;
    while (1 == 1) {
        char c = skip();
        // Token is starting
        token.contents = (Lexer.src + Lexer.index - 1);
        token.debug_info.col = Lexer.col;
        token.debug_info.row = Lexer.row;
        token.contents_len = 1;
        switch(c) {
            case '\0':
                token.kind = T_END;
                return token;
            case '!':
                if ((c = next()) == '=') {
                    token.contents_len = 2;
                    token.kind = T_NOTEQUALS;
                    return token;
                }
                else {
                    putback(c);
                    token.kind = T_XMARK;
                    return token;
                }
            case '&':
                if ((c = next()) == '&') {
                    token.contents_len = 2;
                    token.kind = T_LOGAND;
                    return token;
                }
                else {
                    putback(c);
                    token.kind = T_MOD;
                    return token;
                }
            case '=':
                if ((c = next()) == '=') {
                    token.contents_len = 2;
                    token.kind = T_EQUALITY;
                    return token;
                }
                else {
                    putback(c);
                    token.kind = T_ASSIGN;
                    return token;
                }
            case '-':
                if (isdigit(c = next())) {
                    token.contents_len = scan_int(c);
                    token.kind = T_INTLITERAL;
                    return token;
                }
                else {
                    putback(c);
                    token.kind = T_SUB;
                    return token;
                }
            case '+':
                if((c = next()) == '=') {
                    token.contents_len = 2;
                    //token.kind = T_ADD_INC;
                    return token;
                }
                else {
                    putback(c);
                    token.kind = T_ADD;
                    return token;
                }
            case '*':
                if ((c = next()) == "=") {
                    token.contents_len = 2;
                    //token.kind = T_MUL_INC;
                    return token;
                }
                else {
                    putback(c);
                    token.kind = T_MUL;
                    return token;
                }
            // Single token ones:
            case '(':
            case ')':
            case ';':
            case ',':
            case '{':
            case '}':
                token.kind = char_to_token_type(c);
                return token;
            default:
                if (isdigit(c)) {
                    token.contents_len = scan_int(c);
                    token.kind = T_INTLITERAL;
                    return token;
                }
                else if (isalpha(c) || '_' == c) {
                    token.contents_len = scan_symbol(c);
                    char buffer[16]; // Max length of symbol;
                    strncpy(buffer, token.contents, token.contents_len);
                    buffer[token.contents_len] = '\0';
                    if ((token.kind = keyword(buffer)) != 0) {
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

