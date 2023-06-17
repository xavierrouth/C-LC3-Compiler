#include "parser.h"
#include <stdio.h>

static parser_t Parser; // Initialized to 0

static token_t next_token() 
{
    token_t token;
    if (Parser.putback.kind != T_INVALID) {
        token = Parser.putback;
        Parser.putback.kind = T_INVALID;
        return token;
    }

    token = get_token(); // Get the token from the lexer.

    return token;
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

static void putback_token(token_t t) 
{
    Parser.putback = t;
    return;
}
static ast_node_t* parse_external_declaration() {
    // Function definition or declaration
    // Try to parse delaration_specifiers
    // Returns a statement node
    ast_node_t* node = NULL;
    // Returns a compound statement
    static int i = 0;
    if (i < 5) {
        node = init_ast_node();
        i++;
        node->type = A_INTEGER_LITERAL;
        node->lit_value = i;
    }
    

    return node;
    
}

static ast_node_t* parse_translation_unit() {
    ast_node_t* p = init_ast_node();
    p->type = A_PROGRAM;

    // A bunch of external_declarations
    
    //ast_node_t** node = &(program_node->data.stmt.next); // Get a reference to program_node->next;
    // sets next to parse_external_declaration(), then updates it so the next of that is the next 
    ast_node_t* c = init_compound_statement();
    p->program.cmpd = c;


    ast_node_t* stmt;
    while ((stmt = parse_external_declaration()) != NULL) {
        compound_statement_push(c, stmt);
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

void init_parser() {
    Parser.ast_root = NULL;
    token_t a = {
        .kind = T_INVALID,
        .debug_info = {.col = 1, .row = 1},
        .contents = "Invalid Token",
        .contents_len = sizeof("Invalid Token")
    };
    Parser.putback = a;
    return;
}

ast_node_t* get_root() {
    return Parser.ast_root;
}
