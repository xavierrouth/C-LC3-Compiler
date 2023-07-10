#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"
#include "analysis.h"

#define VARIABLE_DECL true
#define FUNCTION_DECL false

#define MAX_PUTBACK_TOKENS 8

#define errorf printf

static parser_t parser; // parser-scope parser state
static parser_error_handler error_handler; // parser-scope error handler

/** ==================================================
 * Parser Errors:
 * =================================================== */

static void print_error(parser_error_t error);

/* Ends parsing and reports all errors / warnings. */
static void end_parse() {
    static bool triggered = false;

    if (triggered) {
        return;
    }
    // Just the first error:
    /**
    if (error_handler.num_errors > 0)
        print_error(error_handler.errors[0]);
    */
    for (int i = 0; i < error_handler.num_errors; i++) {
        print_error(error_handler.errors[i]);
    }
    for (int i = 0; i < error_handler.num_warnings; i++) {
        //print_error(error_handler.warnings[i]);
    }
    triggered = true;
}

static void report_error(parser_error_t error) {
    if (error_handler.num_errors == MAX_NUM_PARSER_ERRORS) {
        end_parse();
    }
    error_handler.errors[error_handler.num_errors++] = error;
    
}

static void report_warning(parser_error_t error) {
    return;
}

static void print_error(parser_error_t error) {
    token_t previous = error.prev_token;
    size_t len = strlen("Line # | ");
    switch (error.type) {
        
        case ERROR_MISSING_SEMICOLON: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Expected semicolon.\n");
            print_line(previous.debug_info.row, parser.source, parser.source_size);
            printf_indent(previous.debug_info.col + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;
        }
        case ERROR_MISSING_EXPRESSION: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Expected an expression.\n");
            print_line(previous.debug_info.row, parser.source, parser.source_size);
            printf_indent(previous.debug_info.col + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;
        }
        default: {
            printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Something is wrong!.\n");
            print_line(error.prev_token.debug_info.row, parser.source, parser.source_size);
            printf_indent(previous.debug_info.col + len, ANSI_COLOR_GREEN"^\n"ANSI_COLOR_RESET);
            return;
        }
    }
}
    

/** ==================================================
 * Token stream interaction:
 * =================================================== */

static token_t prev_token;

// Gets the next token from the token stream, including tokens that have been putback.
static token_t next_token() 
{   
    static token_t helper_token;
    token_t token;

    // Abort flag has been raised, stop parsing and print all errors.
    if (error_handler.abort) {
        token.kind = T_END;
        end_parse();
        return token;
    }
    
    if (parser.putback_idx > 0) {
        token = parser.putback_stack[--parser.putback_idx];
        // If its putback, then don't update the prev token.
    }
    else {
        token = get_token();
        prev_token = helper_token;
        helper_token = token;
    }
    
    return token;
}

static void skip_statement() {
    // Eat tokens until we get a ';'
    while (true) {
        token_t token = next_token();
        if (token.kind == T_SEMICOLON) {
            break;
        }
        if (token.kind == T_END) {
            end_parse();
            break;
        }
    }
}

static token_t previous_token()
{
    return prev_token;
}

// Do we need to allow for multiple token putback?
static void putback_token(token_t t) 
{
    if (parser.putback_idx == MAX_PUTBACK_TOKENS) {
        printf("Too many putbacks!!");
        return;
    }
    parser.putback_stack[parser.putback_idx++] = t;
    return;
}

static token_t peek_token() {
    token_t t = next_token();
    putback_token(t);
    return t;
}

/* Look ahead at the next token, and return whether it is of a certain type.
This does not consume the token*/
static bool expect_token(token_enum type) {
    token_t t = next_token();

    if (t.kind != type) {
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
        if (type == T_SEMICOLON) {
            parser_error_t error = {
                .invalid_token = t,
                .prev_token = previous_token(),
                .type = ERROR_MISSING_SEMICOLON
            };
            report_error(error);
        }
        else {
            parser_error_t error = {
                .invalid_token = t,
                .prev_token = previous_token(),
                .type = ERROR_GENERAL
            };
            report_error(error);
            
            t.kind = T_INVALID;
        }
        error_handler.abort = true;
        return t;
        
    }
    return t;
}

/** ==================================================
 * Pratt Parsing / Operator Powers:
 * =================================================== */

typedef struct BP_PAIR_STRUCT{
    uint16_t l;
    uint16_t r;
} bp_pair;

// 32 is the number of op types we have.
static uint16_t prefix_binding_power[48];
static bp_pair infix_binding_power[48];
static uint16_t postfix_binding_power[48];

static bool is_infix(ast_op_enum op) {
    return (infix_binding_power[op].l != 0 && infix_binding_power[op].r != 0);
}

static bool is_prefix(ast_op_enum op) {
    return (prefix_binding_power[op] != 0);
}

static bool is_postfix(ast_op_enum op) {
    return (postfix_binding_power[op] != 0);
}

// https://en.cppreference.com/w/c/language/operator_precedence
static void init_infix_binding_power() {
    infix_binding_power[OP_MUL] = (bp_pair) {.l = 26, .r = 27};
    infix_binding_power[OP_DIV] = (bp_pair) {.l = 26, .r = 27};
    infix_binding_power[OP_MOD] =  (bp_pair) {.l = 26, .r = 27};
    infix_binding_power[OP_ADD] = (bp_pair) {.l = 24, .r = 25};
    infix_binding_power[OP_SUB] = (bp_pair) {.l = 24, .r = 25};
    infix_binding_power[OP_LEFTSHIFT] = (bp_pair) {.l = 22, .r = 23};
    infix_binding_power[OP_RIGHTSHIFT] = (bp_pair) {.l = 22, .r = 23};
    
    infix_binding_power[OP_GT_EQUAL] = (bp_pair) {.l = 14, .r = 15};
    infix_binding_power[OP_LT_EQUAL] = (bp_pair) {.l = 14, .r = 15};
    infix_binding_power[OP_GT] = (bp_pair) {.l = 14, .r = 15};
    infix_binding_power[OP_LT] = (bp_pair) {.l = 14, .r = 15};

    infix_binding_power[OP_EQUALS] = (bp_pair) {.l = 12, .r = 13};
    infix_binding_power[OP_NOTEQUALS] = (bp_pair) {.l = 12, .r = 13};

    infix_binding_power[OP_BITAND] = (bp_pair) {.l = 10, .r = 11};
    infix_binding_power[OP_BITXOR] = (bp_pair) {.l = 8, .r = 9};

    infix_binding_power[OP_ASSIGN] = (bp_pair) {.l = 5, .r = 4};

    
}

static void init_prefix_binding_power() {
    prefix_binding_power[OP_ADD] = 28;
    prefix_binding_power[OP_SUB] = 28;
    prefix_binding_power[OP_BITAND] = 28;
    prefix_binding_power[OP_INCREMENT] = 28;
}

static void init_postfix_binding_power() {
    postfix_binding_power[OP_INCREMENT] = 28;
    postfix_binding_power[OP_DECREMENT] = 28;
    // Array access, function call, struct member access, ptr dereference
}

static void init_binding_power() {
    init_infix_binding_power();
    init_prefix_binding_power();
    init_postfix_binding_power();
}


// TODO: AND can become address or bitAND
static ast_op_enum get_op(const token_enum type) {
    switch(type) {
    case T_ASSIGN: return OP_ASSIGN;
    case T_ADD: return OP_ADD;
    case T_SUB: return OP_SUB;
    case T_MUL: return OP_MUL;
    case T_DIV: return OP_DIV;
    case T_MOD: return OP_MOD;
    case T_LOGAND: return OP_LOGAND;
    case T_LOGOR: return OP_LOGOR;
    case T_LOGNOT: return OP_LOGNOT;
    case T_INCREMENT: return OP_INCREMENT;
    case T_DECREMENT: return OP_DECREMENT;
    case T_BITAND: return OP_BITAND;
    case T_BITOR: return OP_BITOR;
    case T_BITXOR: return OP_BITXOR;
    case T_BITFLIP: return OP_BITFLIP;
    case T_LEFTSHIFT: return OP_LEFTSHIFT;
    case T_RIGHTSHIFT: return OP_RIGHTSHIFT;
    case T_LT: return OP_LT;
    case T_GT: return OP_GT;
    case T_LT_EQUAL: return OP_LT_EQUAL;
    case T_GT_EQUAL: return OP_GT_EQUAL;
    case T_NOTEQUALS: return OP_NOTEQUALS;
    case T_EQUALS: return OP_EQUALS;
    case T_ARROW: return OP_ARROW;
    case T_TERNARY: return OP_TERNARY;
    case T_ASSIGN_LSHIFT: return OP_ASSIGN_LSHIFT;
    case T_ASSIGN_RSHIFT: return OP_ASSIGN_RSHIFT;
    case T_ASSIGN_MUL: return OP_ASSIGN_MUL;
    case T_ASSIGN_ADD: return OP_ASSIGN_ADD;
    case T_ASSIGN_SUB: return OP_ASSIGN_SUB;
    case T_ASSIGN_DIV: return OP_ASSIGN_DIV;
    case T_ASSIGN_MOD: return OP_ASSIGN_MOD;
    case T_ASSIGN_BITAND: return OP_ASSIGN_BITAND;
    case T_ASSIGN_BITXOR: return OP_ASSIGN_BITXOR;
    case T_ASSIGN_BITOR: return OP_ASSIGN_BITOR;
    case T_LPAREN: return OP_LPAREN;
    case T_RPAREN: return OP_RPAREN;
    case T_LBRACKET: return OP_LBRACKET;
    case T_RBRACKET: return OP_RBRACKET;
    case T_COLON: return OP_COLON;
    case T_COMMA: return OP_COMMA;
    default:
        return OP_INVALID;
    }
    return -1;
}

/** ==================================================
 * Parser Body:
 * =================================================== */

static ast_node_t parse_declaration();

static ast_node_t parse_expression(uint16_t min_binding_power);

static ast_node_t parse_var_declaration(token_t id_token, type_info_t type_info);

// Eats the braces:
static ast_node_t parse_compound_statement();

/** If the expression is 'missing' i.e. == -1, then report an error and return true, otherwise return false*/
static bool check_missing_expression(ast_node_t expr) {
    if (expr == -1) {
        parser_error_t error = {
            .invalid_token = get_token(),
            .prev_token = previous_token(),
            .type = ERROR_MISSING_EXPRESSION
        };
        report_error(error);
        return true;
    }

    return false;
} 

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
        if (!expect_token(T_COMMA)) {
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
    ast_node_t symbol = ast_expr_symbol_init(id.contents);

    // Function Call
    if (expect_token(T_LPAREN)) {
        ast_node_t func = parse_function_call(symbol); // Eats parens.
        return func;
    }
    // Just a symbol ref
    return symbol;
}

// Pratt Parsing: https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
// Don't eat semicolons
static ast_node_t parse_expression(uint16_t min_binding_power) {
    ast_node_t left = -1;
    ast_node_t right = -1;

    token_t op_token = peek_token();
    ast_op_enum op_type = get_op(op_token.kind);
    // Match the first token in the expression.
    // It can either be identifier, literal, paren groupings, or prefix op
    if (expect_token(T_INTLITERAL))
        left = parse_int_literal();
    else if (expect_token(T_IDENTIFIER)) {
        left = parse_symbol_ref();
    }
    else if (expect_token(T_LPAREN)) {
        eat_token(T_LPAREN);
        left = parse_expression(0);
        eat_token(T_RPAREN);
    }
    else if (is_prefix(op_type)) {
        // Prefix Expression:
        eat_token(op_token.kind);
        uint16_t op_power = prefix_binding_power[op_type];

        ast_node_t child = parse_expression(op_power);
        left = ast_unary_op_init(op_type, child, PREFIX);
    }
    if (left == -1) {
        parser_error_t error = {
            .prev_token = previous_token(),
            .invalid_token = op_token, 
            .type = ERROR_MISSING_EXPRESSION
        };
        report_error(error);
        // Probably should skip the entire expression.
        skip_statement();
        return -1;
    }

    // Infix Expressions:
    /** Consume tokens until there is a token whose binding power is equal or lower than rbp*/
    while (true) {
        // These are things that definetly mark the end of an expression.
        if (expect_token(T_SEMICOLON) || expect_token(T_RPAREN)) {
            break;
        }
        // These actually can be in expressions, just need to know that if its an expression inside of a function call,
        // need to differentiate between comma tokens as the operator and comma tokens as the separator. 
        // TODO: Support comma operators
        else if(expect_token(T_COMMA)) {
            break;
        }
        
        // At this point we expect a binary op or a postfix op
        op_token = peek_token();
        op_type = get_op(op_token.kind);

        if (op_type == OP_INVALID) {
            // We need an op here.
            parser_error_t error = {
                .prev_token = previous_token(),
                .invalid_token = op_token, 
                .type = ERROR_MISSING_SEMICOLON
            };
            //error_handler.in_construction = error; Not actaully needed 
            report_error(error);
            // Probably should skip the entire expression.
            skip_statement();
            return -1;
        }

        if (is_postfix(op_type)) {
            uint16_t op_power = postfix_binding_power[op_type];
            if (op_power < min_binding_power) {
                break;
            }
            next_token();
            // TODO: Test for '['
            //ast_node_t child = parse_expression(0);
            left = ast_unary_op_init(op_type, left, POSTFIX);
            continue;
        }

        if (is_infix(op_type)) {
            bp_pair op_power =  infix_binding_power[op_type];
            if (op_power.l < min_binding_power) {
                break;
            }
            next_token();
            // TODO: Test for ternary '?' then ':'
            right = parse_expression(op_power.r);
            left = ast_binary_op_init(op_type, left, right); // node =
            continue;
        }
        // Probably an error ehre:?
        parser_error_t error = {
            .prev_token = previous_token(),
            .invalid_token = op_token, 
            .type = ERROR_MISSING_SOMETHING
        };
        report_error(error);
        // Probably should skip the entire expression.
        //skip_statement();
        break;

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

// Eats semicolon
static ast_node_t parse_for_init_clause() {
    type_info_t type_info = parse_declaration_specifiers(); 
    // TODO: Make sure these are only auto, register, and a type.

    if (type_info.type != NOTYPE) {
        if (expect_token(T_IDENTIFIER)) {
            token_t id_token = eat_token(T_IDENTIFIER);
            return parse_var_declaration(id_token, type_info); // This eats semicolon
        }
    }
    
    //else go into pratt parsing():
    
    ast_node_t expr_stmt = parse_expression(0);
    eat_token(T_SEMICOLON);
    return expr_stmt;
}

static ast_node_t parse_for_statement() {
    eat_token(T_FOR);
    eat_token(T_LPAREN);
    // TODO: Only certain declaration specifiers are allowed.
    ast_node_t initializer = parse_for_init_clause(); // May be a expression or a declaration
    ast_node_t condition = parse_expression(0);
    if (condition == -1) {
        condition = ast_int_literal_init(1);
    }
    eat_token(T_SEMICOLON);
    ast_node_t update = parse_expression(0);
    eat_token(T_RPAREN);
    ast_node_t body = parse_compound_statement();
    ast_node_t node = ast_for_stmt_init(initializer, condition, update, body);
    return node;
}

static ast_node_t parse_while_statement() {
    eat_token(T_WHILE);
    eat_token(T_LPAREN);
    ast_node_t condition = parse_expression(0);
    if (check_missing_expression(condition)) {
        skip_statement();
        return -1;
    }
    eat_token(T_RPAREN);
    ast_node_t body = parse_compound_statement();
    return ast_while_stmt_init(condition, body);
}

static ast_node_t parse_if_statement() {
    // TODO: Only support
    eat_token(T_IF);
    eat_token(T_LPAREN);
    ast_node_t condition = parse_expression(0);
    ast_node_t if_stmt = -1;
    ast_node_t else_stmt = -1;
    eat_token(T_RPAREN);

    if (expect_token(T_LBRACE)) {
        if_stmt = parse_compound_statement();
    }
    else {
        // Can be a statement just not a declaration?? wtf.
        if_stmt = parse_expression(0);
        eat_token(T_SEMICOLON);
    }
    // Else part
    if (expect_token(T_ELSE)) {
        eat_token(T_ELSE);
        if (expect_token(T_LBRACE)) {
            else_stmt = parse_compound_statement();
        }
        else {
            else_stmt = parse_expression(0);
            eat_token(T_SEMICOLON);
        }
    }
    ast_node_t node = ast_if_stmt_init(condition, if_stmt, else_stmt);
    return node;
}



// Parse a statement that is in a function.
static ast_node_t parse_statement() {
    // TODO: Error checks:
    if (error_handler.abort) {
        return -1;
    }
    // Attempt statement 
    if (expect_token(T_RETURN)) {
        return parse_return_statement();
    }
    //else if (expect_token(T_SWITCH)) {
    //    return a
    //}
    else if (expect_token(T_IF)) {
        return parse_if_statement();
    }
    else if (expect_token(T_WHILE)) {
        return parse_while_statement();
    }
    else if (expect_token(T_FOR)) {
        return parse_for_statement();
    }

    // Attempt var declaration/
    type_info_t type_info = parse_declaration_specifiers(); 

    if (type_info.type != NOTYPE) {
        if (expect_token(T_IDENTIFIER)) {
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
    if (expect_token(T_ASSIGN)) {
        eat_token(T_ASSIGN);
        // Parse variable initialization definition
        ast_node_t initializer = parse_expression(0);
        if (check_missing_expression(initializer)) {
            skip_statement(); 
            return -1;
        }
        eat_token(T_SEMICOLON);
        ast_node_t node = ast_var_decl_init(initializer, type_info, id_token.contents);
        return node;
    }
    else {
        eat_token(T_SEMICOLON); 
        ast_node_t node = ast_var_decl_init(-1, type_info, id_token.contents);
        return node;
    }
}

static ast_node_t parse_compound_statement() {    
    eat_token(T_LBRACE);
    // Parse compound statement; // EATS THE BRACES
    ast_node_vector statements = ast_node_vector_init(16);
    while(true) {
        ast_node_t stmt = parse_statement();
        if (error_handler.abort) {
            ast_node_vector_free(statements);
            return -1;
        }
        ast_node_vector_push(&(statements), stmt);
        if (expect_token(T_RBRACE)) {
            break;
        }
    }

    eat_token(T_RBRACE);
    ast_node_t node = ast_compound_stmt_init(statements, NEWSCOPE);
    return node;
}


static ast_node_t parse_declaration() {
    // Function definition or declaration
    if (error_handler.abort) {
        return -1;
    }
    if (expect_token(T_END)) {
        eat_token(T_END);
        return -1;
    }

    // OR can be structs, unions, enums, typdefs, ETC.
    // Switch over these Tokens ^^:

    type_info_t type_info = parse_declaration_specifiers();

    if (expect_token(T_MUL)) {
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
    if (expect_token(T_LPAREN)) {
        ast_node_t node; // Unintiialized.
        eat_token(T_LPAREN);
        // Parse function declaration
        ast_node_vector parameters = ast_node_vector_init(6);

        // While the next token isn't a Rparen, parse parameters
        while (!expect_token(T_RPAREN)) {
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
            if (!expect_token(T_COMMA))
                break;
            eat_token(T_COMMA);
        } // Done parsing parameters.

        eat_token(T_RPAREN);

        // Function without a body.
        if (expect_token(T_SEMICOLON)) {
            // Body is unitiliazed.
            // TODO: Functions withouts bodies are not supported yet.
            // ie definitions but not declarations
            errorf("Function not allowed without body.\n");
            return -1;
        }
        // Function with a body.
        else if (expect_token(T_LBRACE)){

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
    parser.ast_root = parse_translation_unit();
    return;
}

void teardown_ast() {
    // Traverse the AST and free it.
    free_ast(parser.ast_root);
    return;
}

void init_parser(const char* source, uint32_t source_size) {
    parser.ast_root = -1;
    parser.source = source;
    parser.source_size = source_size;
    init_binding_power();
    return;
}

ast_node_t get_root() {
    return parser.ast_root;
}
