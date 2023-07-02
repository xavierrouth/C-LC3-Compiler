#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"
#include "analysis.h"

#define VARIABLE_DECL true
#define FUNCTION_DECL false

#define errorf printf

static parser_t Parser; // Initialized to 0

/** ==================================================
 * Token stream interaction:
 * =================================================== */

// Gets the next token from the token stream, including tokens that have been putback.
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

static token_t peek_token() {
    token_t t = next_token();
    putback_token(t);
    return t;
}

/* Look ahead at the next token, and return whether it is of a certain type.
This does not consume the token*/
static bool expect_token(token_enum type, bool print_error) {
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

/** ==================================================
 * Pratt Parsing / Operator Powers:
 * =================================================== */

// 32 is the number of op types we have.
static uint16_t prefix_binding_power[32];
static uint16_t infix_binding_power[32];
static uint16_t postfix_binding_power[32];

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
    prefix_binding_power[OP_INC] = 15;
}

static void init_postfix_binding_power() {
    postfix_binding_power[OP_INC] = 15;
    postfix_binding_power[OP_DEC] = 15;
    // Array access, function call, struct member access, ptr dereference
}

static void init_binding_power() {
    init_infix_binding_power();
    init_prefix_binding_power();
    init_postfix_binding_power();
}

static ast_op_enum token_type_to_op(token_enum type) {
    switch(type) {
        case T_ADD: return OP_ADD;
        case T_SUB: return OP_SUB;
        case T_DIV: return OP_DIV;
        case T_MUL: return OP_MUL;
    }
    return -1;
}

static bool is_prefix_op(ast_op_enum op) {
    switch (op) {
        case OP_ADD: 
        case OP_SUB:
        case OP_MUL:
        case OP_NOT:
        case OP_BITNOT:
        case OP_DEC:
        case OP_INC:
            return true;
        default:
            return false;
    }
}

/** ==================================================
 * Praser Body:
 * =================================================== */

static ast_node_t parse_declaration();

static ast_node_t parse_expression(int binding_power);

static ast_node_t parse_var_declaration(token_t id_token, type_info_t type_info);

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



static ast_node_t parse_int_literal() {
    token_t t = eat_token(T_INTLITERAL);
    ast_node_t node = ast_int_literal_init(atoi(t.contents));
    return node;
}

static ast_node_t parse_function_call(ast_node_t symbol_ref) {

    eat_token(T_LPAREN);
    ast_node_vector arguments = ast_node_vector_init(4);
    
    // Need to make sure NOT to eat the last , operator.
    // This is difficult!
    while (true) {
        ast_node_t arg = parse_expression(0);
        ast_node_vector_push(&(arguments), arg);
        if (!expect_token(T_COMMA, false)) {
            // Error Here:
            break;
        }
        eat_token(T_COMMA);
    }
    eat_token(T_RPAREN);
    ast_node_t node = ast_expr_call_init(symbol_ref, arguments);

    return node;
}

// function call can be turned into an operator, and symbol ref should be turend into its own node.
static ast_node_t parse_symbol_ref() {
    token_t id = eat_token(T_IDENTIFIER);
    // Scope::
    ast_node_t symbol = ast_expr_symbol_init(id.contents, 0);

    // Function Call
    if (expect_token(T_LPAREN, false)) {
        ast_node_t func = parse_function_call(symbol); // Eats parens.
        return func;
    }
    // Just a symbol ref
    return symbol;
}

// Pratt Parsing: https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
// Don't eat semicolons
static ast_node_t parse_expression(int binding_power) {
    ast_node_t left = -1;
    ast_node_t right = -1;

    int left_power = 0;
    int right_power = 0;

    // Match the first token in the expression.
    // It can either be identifier, literal, paren groupings,
    token_t op_token = peek_token();
    ast_op_enum op_type = token_type_to_op(op_token.kind);

    if (expect_token(T_INTLITERAL, false))
        left = parse_int_literal();
    else if (expect_token(T_IDENTIFIER, false)) {
        left = parse_symbol_ref();
    }
    else if (expect_token(T_LPAREN, false)) {
        eat_token(T_LPAREN);
        left = parse_expression(0);
        eat_token(T_RPAREN);
    }
    else if (is_prefix_op(op_type)) {
        // Prefix Expression:
        eat_token(op_token.kind);
        // Don't change left power.
        right_power = prefix_binding_power[op_type];
        ast_node_t child = parse_expression(right_power);
        left = ast_unary_op_init(op_type, child, PREFIX);
    }

    // Infix Expressions:
    /** Consume tokens until there is a token whose binding power is equal or lower than rbp*/
    while (true) {
        // These are things that definetly mark the end of an expression.
        if (expect_token(T_SEMICOLON, false) || expect_token(T_RPAREN, false)) {
            break;
        }
        // These actually can be in expressions, just need to know that if its an expression inside of a function call,
        // need to differentiate between comma tokens as the operator and comma tokens as the separator. 
        // TODO: Support comma operators
        else if(expect_token(T_COMMA, false)) {
            break;
        }
        
        // At this point we expect a binary op or a postfix op
        op_token = peek_token();
        op_type = token_type_to_op(op_token.kind);
        
        left_power = infix_binding_power[op_type];
        right_power = infix_binding_power[op_type] + 1;
                
        if (left_power < binding_power) {
            break;
        }

        next_token();
        right = parse_expression(right_power);
        left = ast_binary_op_init(op_type, left, right); // node =

        /** What do we do here?*/
        //left = node;
    } 
    return left;
}

static ast_node_t parse_return_statement() {
    // Error checking:
    eat_token(T_RETURN);
    ast_node_t expression = parse_expression(0);
    ast_node_t node = ast_return_stmt_init(expression);
    eat_token(T_SEMICOLON);
    return node;
}

static ast_node_t parse_if_statement() {
    return -1;
}

// Parse a statement that is in a function.
static ast_node_t parse_statement() {
    // TODO: Error checks:
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
            return parse_var_declaration(id_token, type_info); // This eats semicolon
        }
    }
    
    //else go into pratt parsing():
    
    ast_node_t expr_stmt = parse_expression(0);
    eat_token(T_SEMICOLON);
    return expr_stmt;
}

static ast_node_t parse_var_declaration(token_t id_token, type_info_t type_info) {

    // TODO: Implement multiple variable initialization.
    
    if (expect_token(T_SEMICOLON, false)) {
        eat_token(T_SEMICOLON); 
        ast_node_t node = ast_var_decl_init(-1, type_info, id_token.contents);
        return node;
    }
    else if (expect_token(T_ASSIGN, false)) {
        eat_token(T_ASSIGN);
        // Parse variable initialization definition
        ast_node_t initializer = parse_expression(0);
        eat_token(T_SEMICOLON);
        ast_node_t node = ast_var_decl_init(initializer, type_info, id_token.contents);
        return node;
    }

    // TODO: Error:
    errorf("Expecting variable declaration.\n");
    return -1;
}

static ast_node_t parse_compound_statement() {    
    eat_token(T_LBRACE);
    // Parse compound statement; // EATS THE BRACES
    ast_node_vector statements = ast_node_vector_init(16);
    while(true) {
        ast_node_t stmt = parse_statement();
        if (stmt == -1) {
            break;
        }
        ast_node_vector_push(&(statements), stmt);
        if (expect_token(T_RBRACE, false)) {
            break;
        }
    }

    eat_token(T_RBRACE);
    ast_node_t node = ast_compound_stmt_init(statements, NEWSCOPE);
    return node;
}


static ast_node_t parse_declaration() {
    // Function definition or declaration
    if (expect_token(T_END, false)) {
        eat_token(T_END);
        return -1;
    }

    // OR can be structs, unions, enums, typdefs, ETC.
    // Switch over these Tokens ^^:

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
        return -1;
    }

    // It can either be a var declaration or a function delcaration.
    // Function Declaration:
    
    // Function Declaration:
    if (expect_token(T_LPAREN, false)) {
        ast_node_t node; // Unintiialized.
        eat_token(T_LPAREN);
        // Parse function declaration
        ast_node_vector parameters = ast_node_vector_init(6);

        // While the next token isn't a Rparen, parse parameters
        while (!expect_token(T_RPAREN, false)) {
            // Expect a type (int only for now)
            eat_token(T_INT);
            type_info_t param_type_info = {0};
            param_type_info.type = INT;

            // Expect a name
            token_t param_id_token = eat_token(T_IDENTIFIER);
            ast_node_t parameter = ast_param_decl_init(param_type_info, param_id_token.contents);

            ast_node_vector_push(&(parameters), parameter);
            // Expect a comma maybe
            // If there isn't a comma, then break.
            if (!expect_token(T_COMMA, false))
                break;
            eat_token(T_COMMA);
        } // Done parsing parameters.

        eat_token(T_RPAREN);

        // Function without a body.
        if (expect_token(T_SEMICOLON, false)) {
            // Body is unitiliazed.
            // TODO: Functions withouts bodies are not supported yet.
            // ie definitions but not declarations
            errorf("Function not allowed without body.\n");
            return -1;
        }
        // Function with a body.
        else if (expect_token(T_LBRACE, false)){

            ast_node_t body = parse_compound_statement();
            node = ast_func_decl_init(body, parameters, type_info, id_token.contents);
            return node;
        }
    } // End function declaration.

    // Variable Declaration:
    else {
        ast_node_t node = parse_var_declaration(id_token, type_info);
        return node;
    }
    // Something went wrong:
    return -1;
}


static ast_node_t parse_translation_unit() {

    // A bunch of external_declarations
    
    ast_node_vector body = ast_node_vector_init(16);

    while(true) {
        ast_node_t decl = parse_declaration();
        if (decl == -1) {
            break;
        }
        ast_node_vector_push(&(body), decl);
    }

    ast_node_t node = ast_program_init(body);

    return node;
}

void build_ast() {
    Parser.ast_root = parse_translation_unit();
    return;
}

void teardown_ast() {
    // Traverse the AST and free it.
    free_ast(Parser.ast_root);
    return;
}

void init_parser(bool error_mode) {
    Parser.ast_root = -1;
    Parser.error_mode = error_mode;
    init_binding_power();
    return;
}

ast_node_t get_root() {
    return Parser.ast_root;
}
