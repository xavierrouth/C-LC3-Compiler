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

/* Look ahead at the next token, and return whether it is of a certain type.
This does not consume the token*/
static bool expect_token(token_enum type, bool print_error)
{
    token_t t = next_token();

    if (t.kind != type) {
        if (print_error == true) {
            if (type == T_SEMICOLON) {
                printf("Expected semicolon at end of line %d.\n", t.debug_info.row);
            }
        }
        putback_token(t);
        return false;
    }
    else {
        putback_token(t);
        return true;
    }
}

// Helper function to just eat a token of a certain type, and return the contents.
// Always errors if the token is not the correct type.
static token_t eat_token(token_enum type)
{
    token_t t = next_token();
    if (t.kind != type) {
        print_token(&t);
        if (type == T_SEMICOLON) {
            printf("Expected semicolon at end of line %d.\n", t.debug_info.row);
        }
        else {
            printf("Error: Eat Token Unexpected token encountered.\n");
            t.kind = T_INVALID;
        }
    }
    return t;
}

static bool copy_token_to_id(token_t id_token, ast_node_t* node) {
    if (id_token.contents_len > 16) {
        printf("Error, identifier is too long.");
        return false;
    }
    // Always but identifier first in struct and maybe it just works??
    // Is this UB?
    // Might have to have a switch (node->type) to assign it to union correctly.

    memcpy(node->as.var_decl.identifier, id_token.contents, id_token.contents_len);
    // Null terminate the contents string so we can print it easier later
    node->as.var_decl.identifier[id_token.contents_len] = '\0';
    return true;
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
        int negative = 1;

        if (buf[0] == '-') {
            negative = -1;
            buf++;
        }

        while (len--)
                n = n*10 + *buf++ - '0';

        return n * negative;
}

static ast_node_t* parse_int_literal() {
    ast_node_t* node = init_ast_node();
    // For now just handle single int literals LOL;
    token_t t = eat_token(T_INTLITERAL);
    node->type = A_INTEGER_LITERAL;
    node->as.literal.value = len_atoi(t.contents, t.contents_len);
    return node;
}

static ast_node_t* parse_function_call() {
    // TODO: Implement this
    printf("Parsing Func Call w/ no arguments");
    eat_token(T_LPAREN);
    eat_token(T_RPAREN);
    return NULL;
}

static ast_node_t* parse_primary_expression();

/** TODO: Pratt parsing here:*/
static ast_node_t* parse_expression(int precedence) {
    ast_node_t* node;
    if (expect_token(T_ADD, false)) {
        eat_token(T_ADD);
        node = init_ast_node();
        node->type = A_BINOP_EXPR;
        node->as.binary_op.type = OP_ADD;
        node->as.binary_op.left = NULL;
    }
    node = parse_primary_expression();
    return node;
}

/** This is the lowest level of expression*/
static ast_node_t* parse_primary_expression() {
    ast_node_t* node;
    if (expect_token(T_LPAREN, false)) {
        eat_token(T_LPAREN);
        node = parse_expression(0);
        eat_token(T_RPAREN); 
    }
    else if (expect_token(T_IDENTIFIER, false)) {
        token_t id_token = eat_token(T_IDENTIFIER);
        // Check if its a function call next
        if (expect_token(T_LPAREN, false)) {
            node = parse_function_call();
            // Parse function call
        }
        else {
            node = init_ast_node();
            node->type = A_VAR_EXPR;
            copy_token_to_id(id_token, node);
            node->as.var_ref_expr.scope = 0; // Everything global scope for now?
        }
    }
    else if (expect_token(T_INTLITERAL, false)) {
        node = parse_int_literal();
    }

    return node;
}

static ast_node_t* parse_declaration() {
    // Function definition or declaration
    if (expect_token(T_END, false)) {
        eat_token(T_END);
        return NULL;
    }

    type_info_t type_info = parse_declaration_specifiers();

    if (expect_token(T_MUL, false)) {
        eat_token(T_MUL);
        type_info.is_pointer = true;
    }

    // We always need an identifier here:
    // If not, we need to do error handling somehow.
    // For now, store error in the token type.
    token_t id_token = eat_token(T_IDENTIFIER);
    if (id_token.kind == T_INVALID) {
        printf("Invalid token encountered, aborting AST building.\n");
        return NULL;
    }
    // Do we need multiple cases if we use two differnet unions 
    // even though the memory layout is the same?
    ast_node_t* node = init_ast_node();
    node->as.var_decl.initializer = NULL;
    node->as.var_decl.type_info = type_info;
    // Copies the token contents to the node identifier.
    copy_token_to_id(id_token, node);

    // TODO: Implement multiple variable initialization.
    
    if (expect_token(T_SEMICOLON, false)) {
        eat_token(T_SEMICOLON);
        node->type = A_VAR_DECL;
        node->as.var_decl.type_info = type_info;
        return node;
    }
    else if (expect_token(T_ASSIGN, false)) {
        eat_token(T_ASSIGN);
        // Parse variable initialization definition
        node->type = A_VAR_DECL;
        node->as.var_decl.type_info = type_info;
        node->as.var_decl.initializer = parse_primary_expression();
        eat_token(T_SEMICOLON);
        return node;
    }
    else if (expect_token(T_LPAREN, false)) {
        eat_token(T_LPAREN);
        // Parse function declaration
        node->type = A_FUNCTION_DECL;
        eat_token(T_RPAREN);
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
