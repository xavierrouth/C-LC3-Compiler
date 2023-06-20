#include <stdio.h>
#include <stdbool.h>

#include "parser.h"
#include "type_table.h"

#define VARIABLE_DECL true
#define FUNCTION_DECL false

static parser_t Parser; // Initialized to 0

static token_t next_token() 
{
    token_t token;
    if (Parser.putback_idx > 0)
        return Parser.putback_stack[--Parser.putback_idx];
    
    return get_token();
}

// Do we need to allow for multiple token putback?
static void putback_token(token_t t) 
{
    if (Parser.putback_idx == 7) {
        printf("Too many putbacks!!");
        return;
    }
    Parser.putback_stack[Parser.putback_idx++] = t;
    return;
}

static bool expect_token(token_enum type)
{
    token_t t = next_token();

    if (t.kind != type) {
        if (Parser.error_mode == true) {
            if (type == T_SEMICOLON) {
                printf("Expected semicolon at end of line %d.\n", t.debug_info.row);
            }
        }
        putback_token(t);
        return false;
    }
    else {
        return true;
    }
}

// Helper function to just eat a token, and don't do anything with its contents.
static token_t eat_token(token_enum type)
{
    token_t t = next_token();
    if (t.kind != type) {
        printf("Unexpected token encountered.\n");
    }
    return t;
}



// Give it a node 
static type_info_t parse_declaration_specifiers() {
    // Storage Class:
    // typedef, extern, static, auto, register
    // Type Specifier:
    // void, char, short, int, long, float, double, signed, unsigned, struct, union, enum, type_name
    // Type Qualifier:
    // const, volatile
    token_t t;
    type_info_t type_info = {0};
    
    while((t = next_token()).kind != T_END) {
        switch (t.kind) {
            /**
             * If the token is not a type specifier, then we are done.
             * We put the token back and return the type info that we have collected so far.
            */
            case T_TYPEDEF:
            case T_EXTERN:
            case T_STATIC:
            //case T_AUTO:
            //case T_REGISTER:
            case T_VOID:
                type_info.type = VOID;
                continue;
            case T_CHAR:
            //case T_SHORT:
            case T_INT:
                type_info.type = INT;
                continue;
            //case T_LONG:
            //case T_FLOAT:
            //case T_DOUBLE:
            //case T_SIGNED:
            //case T_UNSIGNED:
            case T_STRUCT:
            case T_UNION:
            case T_ENUM:
                continue;
            default:
               goto end;
        }
    }
    end: 
    putback_token(t);
    return type_info;
}

// digits only
int len_atoi(const char *buf, size_t len)        
{
        int n=0;

        while (len--)
                n = n*10 + *buf++ - '0';

        return n;
}

static ast_node_t* parse_int_literal() {
    ast_node_t* node = init_ast_node();
    // For now just handle single int literals LOL;
    token_t t = get_token();
    node->type = A_INTEGER_LITERAL;
    node->as.literal.value = len_atoi(t.contents, t.contents_len);
    return node;
}

static ast_node_t* parse_declaration() {
    // Function definition or declaration
    if (expect_token(T_END)) {
        return NULL;
    }

    ast_node_t* node = init_ast_node();
    type_info_t type_info = parse_declaration_specifiers();

    if (expect_token(T_MUL)) {
        type_info.is_pointer = true;
    }

    token_t id_token = eat_token(T_IDENTIFIER);

    // Do we need multiple cases if we use two differnet unions 
    // even though the memory layout is the same?
    node->as.var_decl.initializer = NULL;
    node->as.var_decl.type_info = type_info;
    if (id_token.contents_len > 16) {
        printf("Error, identifier is too long.");
    }
    memcpy(node->as.var_decl.identifier, id_token.contents, id_token.contents_len);
    // Null terminate the contents string so we can print it easier later
    node->as.var_decl.identifier[id_token.contents_len] = '\0';

    // TODO: Implement multiple variable initialization.
    
    if (expect_token(T_SEMICOLON)) {
        node->type = A_VAR_DECL;
        return node;
    }
    else if (expect_token(T_ASSIGN)) {
        // Parse variable initialization definition
        node->type = A_VAR_DECL;
        node->as.var_decl.type_info = type_info;
        node->as.var_decl.initializer = parse_int_literal();
        expect_token(T_SEMICOLON);
        return node;
    }
    else if (expect_token(T_LPAREN)) {
        // Parse function declaration
        node->type = A_FUNCTION_DECL;
        return node;
    }
    

    return NULL;
    
}



static ast_node_t* parse_translation_unit() {
    ast_node_t* p = init_ast_node();
    p->type = A_PROGRAM;

    // A bunch of external_declarations
    
    p->as.program.body = ast_node_list_init();

    ast_node_t* stmt;
    while ((stmt = parse_declaration()) != NULL) {
        // Add the new stmt to our compound statment;
        ast_node_list_push(&(p->as.program.body), stmt);
    } 

    return p;
}

void build_ast() {
    Parser.ast_root = parse_translation_unit();
    return;
}

void teardown_ast() {
    // Traverse the AST and free it.
    return;
}

void init_parser(bool error_mode) {
    Parser.ast_root = NULL;
    Parser.error_mode = error_mode;
    return;
}

ast_node_t* get_root() {
    return Parser.ast_root;
}
