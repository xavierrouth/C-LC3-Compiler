#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"
#include "type_table.h"

#define VARIABLE_DECL true
#define FUNCTION_DECL false

static parser_t Parser; // Initialized to 0

// Forward decls:
static ast_node_t* parse_declaration();

static ast_node_t* parse_var_declaration(token_t id_token, type_info_t type_info);

static token_t next_token() 
{
    token_t token;
    if (Parser.putback_idx > 0)
        return Parser.putback_stack[--Parser.putback_idx];
    
    return get_token();
}

static int prefix_binding_power[32];
static int infix_binding_power[32];

static void init_infix_binding_power() {
    infix_binding_power[OP_ADD] = 5;
    infix_binding_power[OP_SUB] = 5;
    infix_binding_power[OP_MUL] = 15;
    infix_binding_power[OP_DIV] = 15;
    return;
}

static void init_prefix_binding_power() {
    prefix_binding_power[OP_ADD] = 10;
    prefix_binding_power[OP_SUB] = 10;
}

static void init_binding_power() {
    init_infix_binding_power();
    init_prefix_binding_power();
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

static token_t peek_token() {
    token_t t = next_token();
    putback_token(t);
    return t;
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
    type_info.type = NOTYPE;
    
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
                // TODO: Better error handling
                
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

static ast_op_enum token_type_to_op(token_enum type) {
    switch(type) {
        case T_ADD: return OP_ADD;
        case T_SUB: return OP_SUB;
        case T_DIV: return OP_DIV;
        case T_MUL: return OP_MUL;
    }
}

static ast_node_t* parse_var_expr() {
    expect_token(T_IDENTIFIER, true);
    ast_node_t* node = init_ast_node();
    node->type = A_VAR_EXPR;
    token_t id_token = eat_token(T_IDENTIFIER);
    copy_token_to_id(id_token, node);
    node->as.var_ref_expr.scope = 0;
    return node; 
}
/** TODO: Pratt parsing here:*/
static ast_node_t* parse_expression(int binding_power) {
    
    // Do left token
    ast_node_t* left = NULL;
    ast_node_t* right = NULL;
    ast_node_t* node = NULL;
    int left_power = 0;
    int right_power = 0;
    if (expect_token(T_INTLITERAL, false))
        left = parse_int_literal();
    else if (expect_token(T_IDENTIFIER, false)) {
        left = parse_var_expr();
    }
    // Simplify prefix expressions:
    else if (expect_token(T_LPAREN, false)) {
        eat_token(T_LPAREN);
        left = parse_expression(0);
        eat_token(T_RPAREN);
    }
    else if (expect_token(T_ADD, false)) {
        eat_token(T_ADD);
        left_power = prefix_binding_power[OP_ADD];
        right_power = prefix_binding_power[OP_ADD] + 1;
        right = parse_expression(right_power);
        left = right; // Don't need a special node for this.
    }
    else if (expect_token(T_SUB, false)) {
        eat_token(T_SUB);
        left_power = prefix_binding_power[OP_SUB];
        right_power = infix_binding_power[OP_SUB] + 1;
        right = parse_expression(right_power);
        // Multiply by -1
        ast_node_t* negate_node = init_ast_node();
        negate_node->type = A_INTEGER_LITERAL;
        negate_node->as.literal.value = -1;

        left = init_ast_node();
        left->type = A_BINOP_EXPR;
        left->as.binary_op.type = OP_MUL;
        left->as.binary_op.left = negate_node;
        left->as.binary_op.right = right;
    }

    // Infix Expressions:
    /** Consume tokens until there is a token whose binding power is equal or lower than rbp*/
    while (true) {
        if (expect_token(T_SEMICOLON, false)) {
            //eat_token(T_SEMICOLON);
            break;
        }
        else if(expect_token(T_RPAREN, false)) {
            break;
        }
        // For now just assume that the next token is binop.
        /**
        if (expect_token(T_ADD, false)){
            //eat_token(T_ADD);
            node = init_ast_node();
            node->as.binary_op.type = OP_ADD;
            node->as.binary_op.left = left; 
            node->type = A_BINOP_EXPR; 
        }
        */
        
        node = init_ast_node();
        node->as.binary_op.type = token_type_to_op(peek_token().kind);
        node->as.binary_op.left = left; 
        node->type = A_BINOP_EXPR;
        
        left_power = infix_binding_power[node->as.binary_op.type];
        right_power = infix_binding_power[node->as.binary_op.type] + 1;

        if (left_power < binding_power) {
            break;
        }

        next_token();

        right = parse_expression(right_power);
        node->as.binary_op.right = right;
        
        /** What do we do here?*/
        left = node;
    }
        
    return left;
}

/**
static ast_node_t* parse_expression(int right_binding_power) {
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
*/

// Todo: Add error recovery to everything
static ast_node_t* parse_return_statement() {
    eat_token(T_RETURN);
    ast_node_t* node = init_ast_node();
    node->type = A_RETURN_STMT;
    node->as.return_stmt.expression = parse_expression(0);
    eat_token(T_SEMICOLON);
    return node;
}

static ast_node_t* parse_if_statement() {
    return NULL;
}

static ast_node_t* parse_statement() {
    ast_node_t* node;
    if (expect_token(T_END, false)) {
        eat_token(T_END);
        return NULL;
    }

    // Attempt statement 
    if (expect_token(T_RETURN, false)) {
        return parse_return_statement();
    }
    else if (expect_token(T_IF, false)) {
        return parse_if_statement();
    }

    // Attempt var declaration/
    type_info_t type_info = parse_declaration_specifiers(); 

    if (type_info.type != NOTYPE) {
        if (expect_token(T_IDENTIFIER, false)) {
            token_t id_token = eat_token(T_IDENTIFIER);
            return parse_var_declaration(id_token, type_info); // This eats semicolon I think
        }
    }
    
    // Attempt if statement


    // Attempt loop statement

    // Must be expr statement.

    if (expect_token(T_IDENTIFIER, false)) {
        
        token_t id_token = eat_token(T_IDENTIFIER);
        if (expect_token(T_ASSIGN, false)) {
            node = init_ast_node();
            node->type = A_ASSIGN_EXPR;
            copy_token_to_id(id_token, node);
            eat_token(T_ASSIGN);
            node->as.assign_expr.right = parse_expression(0);
            eat_token(T_SEMICOLON);
            return node;
        }
    }

    return NULL;



}
static ast_node_t* parse_var_declaration(token_t id_token, type_info_t type_info) {
    ast_node_t* node = init_ast_node();
    node->as.var_decl.initializer = NULL;
    node->as.var_decl.type_info = type_info;
    // Copies the token contents to the node identifier.
    copy_token_to_id(id_token, node);

    // TODO: Implement multiple variable initialization.
    
    if (expect_token(T_SEMICOLON, false)) {
        eat_token(T_SEMICOLON); // Don't eat it??/
        node->type = A_VAR_DECL;
        node->as.var_decl.type_info = type_info;
        return node;
    }
    else if (expect_token(T_ASSIGN, false)) {
        eat_token(T_ASSIGN);
        // Parse variable initialization definition
        node->type = A_VAR_DECL;
        node->as.var_decl.type_info = type_info;
        node->as.var_decl.initializer = parse_expression(0);
        eat_token(T_SEMICOLON);
        return node;
    }
    printf("Probably shouldn't get here\n");
    return NULL;
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
        eat_token(T_SEMICOLON); // Don't eat it??/
        node->type = A_VAR_DECL;
        node->as.var_decl.type_info = type_info;
        return node;
    }
    else if (expect_token(T_ASSIGN, false)) {
        eat_token(T_ASSIGN);
        // Parse variable initialization definition
        node->type = A_VAR_DECL;
        node->as.var_decl.type_info = type_info;
        node->as.var_decl.initializer = parse_expression(0);
        eat_token(T_SEMICOLON);
        return node;
    }
    else if (expect_token(T_LPAREN, false)) {
        eat_token(T_LPAREN);
        // Parse function declaration
        node->type = A_FUNCTION_DECL;
        node->as.func_decl.parameters = ast_node_list_init();

        // While the next token isn't a Rparen, parse parameters
        while (!expect_token(T_RPAREN, false)) {
            // Expect a type (int only for now)
            eat_token(T_INT);
            type_info_t type_info = {0};
            type_info.type = INT;

            // Expect a name
            id_token = eat_token(T_IDENTIFIER);
            ast_node_t* param_node = init_ast_node();
            param_node->type = A_VAR_DECL; //
            copy_token_to_id(id_token, param_node);
            
            param_node->as.var_decl.type_info = type_info;
            ast_node_list_push(&(node->as.func_decl.parameters), param_node);
            // Expect a comma maybe
            // If there isn't a comma, then break.
            if (!expect_token(T_COMMA, false))
                break;
            eat_token(T_COMMA);
        }

        eat_token(T_RPAREN);
        // If there is a semicolon, just return this
        if (expect_token(T_SEMICOLON, false)) {
            // Body is unitiliazed.
            // TODO: Functions withouts bodies are not supported yet.
            // ie definitions but not declarations
            return node;
        }
        // Parse function body:
        else if (expect_token(T_LBRACE, false)){
            eat_token(T_LBRACE);
            node->as.func_decl.body = ast_node_list_init();
            ast_node_t* stmt;
            // TOOD: Implement parse_stmt;
            // parse_stmt needs to choose between parse decleration and parse something else
            while ((stmt = parse_statement()) != NULL) {
                ast_node_list_push(&(node->as.func_decl.body), stmt);
                if (peek_token().kind == T_RBRACE)
                    break;
            }
            eat_token(T_RBRACE);
            return node;
        }
    }
    // Can either 
    

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
    init_binding_power();
    return;
}

ast_node_t* get_root() {
    return Parser.ast_root;
}
