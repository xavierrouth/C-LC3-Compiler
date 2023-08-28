#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "error.h"
#include "util/util.h"
#include "parser.h"
#include "analysis.h"

#define VARIABLE_DECL true
#define FUNCTION_DECL false

#define MAX_PUTBACK_TOKENS 8

#define errorf printf

static parser_t parser; // parser-scope parser state
extern parser_error_handler error_handler;

/** ==================================================
 * Parser Errors:
 * =================================================== */

/* Ends parsing and reports all errors / warnings. */
void end_parse() {
    static bool triggered = false;

    if (triggered) {
        return;
    }

    print_errors();
    
    triggered = true;
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
                .invalid_token = previous_token(),
                .prev_token = previous_token(),
                .type = ERROR_MISSING_SEMICOLON,
                .offset = 0
            };
            report_error(error);
        }
        else if (type == T_RPAREN) {
            parser_error_t error = {
                .invalid_token = t,
                .prev_token = previous_token(),
                .type = ERROR_MISSING_RPAREN,
                .offset = 0
            };
            report_error(error);
        }
        else {
            parser_error_t error = {
                .invalid_token = t,
                .prev_token = previous_token(),
                .type = ERROR_UNEXPECTED_TOKEN
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
    infix_binding_power[OP_DOT] = (bp_pair) {.l = 28, .r = 29};
    infix_binding_power[OP_ARROW] = (bp_pair) {.l = 28, .r = 29};

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
    prefix_binding_power[OP_MUL] = 28;
    prefix_binding_power[OP_INCREMENT] = 28;
    prefix_binding_power[OP_DECREMENT] = 28;
}

static void init_postfix_binding_power() {
    postfix_binding_power[OP_LPAREN] = 30;
    postfix_binding_power[OP_LBRACKET] = 30;
    postfix_binding_power[OP_INCREMENT] = 30;
    postfix_binding_power[OP_DECREMENT] = 30;
    
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
    case T_DOT: return OP_DOT;
    default:
        return OP_INVALID;
    }
    return -1;
}

/** ==================================================
 * Parser Body:
 * =================================================== */

static ast_node_h parse_declaration();

static ast_node_h parse_expression(uint16_t min_binding_power);

static declarator_t parse_declarator(type_info_t* type_info, bool check_pointers);

static ast_node_h parse_var_declaration(token_t id_token, type_info_t type_info);

// Eats the braces:
static ast_node_h parse_compound_statement();

/** If the expression is 'missing' i.e. == -1, then report an error and return true, otherwise return false*/
static bool check_missing_expression(ast_node_h expr) {
    if (expr == -1) {
        parser_error_t error = {
            .invalid_token = previous_token(),
            .prev_token = previous_token(),
            .type = ERROR_MISSING_EXPRESSION,
            .offset = 1
        };
        report_error(error);
        return true;
    }
    return false;
} 

static declarator_t parse_pointers() {
    eat_token(T_MUL);
    // Parse type qualifier (const or voltaile ptr)

    declarator_t declarator = {0}; 

    uint16_t idx = declarator.idx;
    if (declarator.idx >= MAX_DECL_PARTS) {
        // Todo: Make this an error
        assert("Too many decl parts nested" && 0);
    }
    declarator.parts[idx].type = POINTER_DECL;
    declarator.idx++;

    if (expect_token(T_MUL)) {
        declarator = merge_declarator(declarator, parse_pointers());
    }

    return declarator;
}

static declarator_t parse_declarator(type_info_t* type_info, bool check_pointers) {
    /**Convert declarator to direct declarator, by conditionally handling pointers*/
    declarator_t right_decl = {0};
    declarator_t left_decl = {0};
    if (check_pointers && expect_token(T_MUL)) {
        right_decl = merge_declarator(right_decl, parse_pointers());
    }

    /** Parse Tokens here*/
    if (expect_token(T_IDENTIFIER)) {
        type_info->identifier_token = eat_token(T_IDENTIFIER); 
    }

    else if (expect_token(T_LPAREN)) {
        eat_token(T_LPAREN);
        left_decl = parse_declarator(type_info, true);
        eat_token(T_RPAREN);
    }

    while (expect_token(T_LBRACKET)) { //| expect_token(T_LPAREN)) {
        if (type_info->identifier_token.kind == T_IDENTIFIER) {
            if (expect_token(T_LBRACKET)) {
                if (left_decl.idx >= MAX_DECL_PARTS) {
                    // Todo: Make this an error
                    assert("Too many decl parts nested" && 0);
                }
                eat_token(T_LBRACKET);
                // TODO: Support different texpressoins for array sizes
                token_t array_size_token = eat_token(T_INTLITERAL);
                uint16_t array_size = atoi(array_size_token.contents);
                eat_token(T_RBRACKET);
                
                left_decl.parts[left_decl.idx].type = ARRAY_DECL;
                left_decl.parts[left_decl.idx].array_size = array_size;
                left_decl.idx++;
            }
        }
        else {
            printf("merme\n");
        }
    }

    return merge_declarator(left_decl, right_decl);

}

// returns true 
static bool parse_declaration_specifier(type_info_t* type_info) {
    // TODO: Support more decl specifiers
    if (expect_token(T_INT)) {
        type_info->specifier_info.is_int = 1;
        eat_token(T_INT);
        return true;
    }
    if (expect_token(T_VOID)) {
        type_info->specifier_info.is_void = 1;
        eat_token(T_VOID);
        return true; 
    }
    if (expect_token(T_CHAR)) {
        type_info->specifier_info.is_char = 1;
        eat_token(T_CHAR);
        return true;
    }
    if (expect_token(T_STATIC)) {
        type_info->specifier_info.is_static = 1;
        eat_token(T_STATIC);
        return true;
    }
    if (expect_token(T_CONST)) {
        type_info->specifier_info.is_const = 1;
        eat_token(T_CONST);
        return true;
    }
    return false;

}

/** Wrapper to loop over declaration specifiers?*/
static type_info_t parse_declaration_specifiers() {
    // Storage Class:
    // typedef, extern, static, auto, register
    // Type Specifier:
    // void, char, short, int, long, float, double, signed, unsigned, struct, union, enum, type_name
    // Type Qualifier:
    // const, volatile
    type_info_t type_info = {0};
    while (parse_declaration_specifier(&type_info)) {
        continue;
    }
    return type_info;
}

static ast_node_h parse_int_literal() {
    token_t t = eat_token(T_INTLITERAL);
    ast_node_h node = ast_int_literal_init(atoi(t.contents));
    return node;
}

static ast_node_h parse_function_call(ast_node_h symbol_ref) {

    eat_token(T_LPAREN);
    ast_node_vector arguments = ast_node_vector_init(4);
    
    if (!expect_token(T_RPAREN)) {
        while (true)  {
            // TODO: Disable parsing comma operators in this. (pass in extra arg to parse_expression)
            ast_node_h arg = parse_expression(0);
            ast_node_vector_push(&(arguments), arg);
            if (!expect_token(T_COMMA)) {
                // Error Here:
                
                break;
            }
            else {
                eat_token(T_COMMA);
            }
        }
    }
    eat_token(T_RPAREN);
    ast_node_h node = ast_expr_call_init(symbol_ref, arguments);

    return node;
}

// function call can be turned into an operator, and symbol ref should be turend into its own node.
static ast_node_h parse_symbol_ref() {
    token_t id = eat_token(T_IDENTIFIER);
    // Scope::
    ast_node_h symbol = ast_expr_symbol_init(id);

    return symbol;
}

// Pratt Parsing: https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
// Don't eat semicolons
static ast_node_h parse_expression(uint16_t min_binding_power) {
    ast_node_h left = -1;
    ast_node_h right = -1;

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
        // Don't allow for empty paren expressions
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
        eat_token(T_RPAREN);
    }
    else if (is_prefix(op_type)) {
        // Prefix Expression:
        eat_token(op_token.kind);
        uint16_t op_power = prefix_binding_power[op_type];

        ast_node_h child = parse_expression(op_power);
        if (check_missing_expression(child))
            return -1;

        left = ast_unary_op_init(op_type, child, PREFIX);
    }
    // TODO: Actually we might want to allow for empty expressions??. i.e return; or ();
    if (left == -1) {
        /**
        parser_error_t error = {
            .prev_token = previous_token(),
            .invalid_token = op_token, 
            .type = ERROR_MISSING_EXPRESSION
        };
        report_error(error);
        // Probably should skip the entire expression.
        skip_statement();
        */
        return -1;
    }

    // Infix Expressions:
    /** Consume tokens until there is a token whose binding power is equal or lower than rbp*/
    while (true) {
        // These are things that definetly mark the end of an expression, some other rule needs to consume them
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

        // Handle 
        if (is_postfix(op_type)) {
            uint16_t op_power = postfix_binding_power[op_type];
            if (op_power < min_binding_power) {
                break;
            }
            
            if (op_type == OP_LPAREN) {
                // Function Call, eats parens
                left = parse_function_call(left);
            }
            else {
                next_token();
                left = ast_unary_op_init(op_type, left, POSTFIX);
            }
            // TODO: Test for '['
            //ast_node_h child = parse_expression(0);
            
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
            if (check_missing_expression(right))
                break;
            left = ast_binary_op_init(op_type, left, right); // node =
            continue;
        }

        // Probably an error ehre:?
        /**
        parser_error_t error = {
            .prev_token = previous_token(),
            .invalid_token = op_token, 
            .type = ERROR_UNEXPECTED_TOKEN
        };
        report_error(error);
        // Probably should skip the entire expression.
        skip_statement();*/
        break;

    } 
    return left;
}

static ast_node_h parse_return_statement() {
    // Error checking:
    eat_token(T_RETURN);
    ast_node_h expression = parse_expression(0);
    ast_node_h node = ast_return_stmt_init(expression);
    eat_token(T_SEMICOLON);
    return node;
}

// Eats semicolon
static ast_node_h parse_for_init_clause() {
    type_info_t type_info = parse_declaration_specifiers(); 
    // TODO: Make sure these are only auto, register, and a type.
    if (type_info.specifier_info.is_int || type_info.specifier_info.is_char) {
        type_info.declarator = parse_declarator(&type_info, true);
        return parse_declaration(type_info); // This eats semicolon
        
    }
    
    // Go into pratt parsing:
    ast_node_h expr_stmt = parse_expression(0);
    eat_token(T_SEMICOLON);
    return expr_stmt;
}

static ast_node_h parse_for_statement() {
    eat_token(T_FOR);
    eat_token(T_LPAREN);
    // TODO: Only certain declaration specifiers are allowed.
    ast_node_h initializer = parse_for_init_clause(); // May be a expression or a declaration
    ast_node_h condition = parse_expression(0);
    if (condition == -1) {
        condition = ast_int_literal_init(1);
    }
    eat_token(T_SEMICOLON);
    ast_node_h update = parse_expression(0);
    eat_token(T_RPAREN);
    ast_node_h body = parse_compound_statement();
    ast_node_h node = ast_for_stmt_init(initializer, condition, update, body);
    return node;
}

static ast_node_h parse_while_statement() {
    eat_token(T_WHILE);
    eat_token(T_LPAREN);
    ast_node_h condition = parse_expression(0);
    if (check_missing_expression(condition)) {
        skip_statement();
        return -1;
    }
    eat_token(T_RPAREN);
    ast_node_h body = parse_compound_statement();
    return ast_while_stmt_init(condition, body);
}

static ast_node_h parse_if_statement() {
    // TODO: Only support
    eat_token(T_IF);
    eat_token(T_LPAREN);
    ast_node_h condition = parse_expression(0);
    ast_node_h if_stmt = -1;
    ast_node_h else_stmt = -1;
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
    ast_node_h node = ast_if_stmt_init(condition, if_stmt, else_stmt);
    return node;
}

static ast_node_h parse_inline_asm() {
    eat_token(T_ASM);
    eat_token(T_LPAREN);
    token_t t = eat_token(T_STRLITERAL);
    eat_token(T_RPAREN);
    ast_node_h node = ast_inline_asm_init(t);
    eat_token(T_SEMICOLON);
    return node;
}

// Parse a statement that is in a function.
static ast_node_h parse_statement() {
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
    else if(expect_token(T_ASM)) {
        return parse_inline_asm();
    }

    // Attempt var declaration/

    type_info_t type_info = parse_declaration_specifiers(); 
    
    if (type_info.specifier_info.is_int || type_info.specifier_info.is_char) {
        type_info.declarator = parse_declarator(&type_info, true);
        return parse_declaration(type_info);
    }

    
    //else go into pratt parsing():
    
    ast_node_h expr_stmt = parse_expression(0);
    eat_token(T_SEMICOLON);
    return expr_stmt;
}

static ast_node_h parse_declaration(type_info_t type_info) {
    // TODO: Implement multiple variable initialization.
    if (expect_token(T_ASSIGN)) {
        eat_token(T_ASSIGN);
        // Parse variable initialization definition
        ast_node_h initializer = parse_expression(0);
        if (check_missing_expression(initializer)) {
            skip_statement(); 
            return -1;
        }
        eat_token(T_SEMICOLON);
        ast_node_h node = ast_var_decl_init(initializer, type_info, type_info.identifier_token);
        return node;
    }
    else {
        eat_token(T_SEMICOLON); 
        ast_node_h node = ast_var_decl_init(-1, type_info, type_info.identifier_token);
        return node;
    }
}

static ast_node_h parse_compound_statement() {    
    eat_token(T_LBRACE);
    // Parse compound statement; // EATS THE BRACES
    ast_node_vector statements = ast_node_vector_init(16);
    while(true) {
        ast_node_h stmt = parse_statement();
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
    ast_node_h node = ast_compound_stmt_init(statements, NEWSCOPE);
    return node;
}



static ast_node_h parse_function_definition(const type_info_t return_type) {
    // 
    token_t id_token = return_type.identifier_token;

    eat_token(T_LPAREN);
    ast_node_h node = -1;
    // Parse parameters

    // TODO: Support Varags
    ast_node_vector parameters = ast_node_vector_init(4);
    
    // Parse paramater-delcaration and then comma
    while (!expect_token(T_RPAREN)) {

        type_info_t param_type = parse_declaration_specifiers();
        // TODO Check if there actualyl was a declaration specifier: Semi implemntation:
        if (!(param_type.specifier_info.is_int || param_type.specifier_info.is_char)) {
            // Error out.
            return -1;
        }
        param_type.declarator = parse_declarator(&param_type, true);
        ast_node_h parameter = ast_param_decl_init(param_type, param_type.identifier_token);

        ast_node_vector_push(&parameters, parameter);
        if (!expect_token(T_COMMA))
            break;
        eat_token(T_COMMA);
    }

    eat_token(T_RPAREN);
    // Parse Body

    if (expect_token(T_SEMICOLON)) {
        // Body is unitiliazed.
        // TODO: Functions withouts bodies are not supported yet.
        // ie definitions but not declarations
        errorf("Function not allowed without body.\n");
        return -1;
    }
    // Function with a body.
    else if (expect_token(T_LBRACE)){
        ast_node_h body = parse_compound_statement();
        node = ast_func_decl_init(body, parameters, return_type, id_token);
        return node;
    }

    return node;

}

static ast_node_h parse_toplevel_declaration() {
    // Parse function declaration or var / normal declaration, who knows what to chosoe! hahahhaha
    if (error_handler.abort) {
        return -1;
    }
    if (expect_token(T_END)) {
        eat_token(T_END);
        return -1;
    }
    if (expect_token(T_ASM)) {
        return parse_inline_asm();
    }

    token_t t = peek_token();
    type_info_t type_info = parse_declaration_specifiers();
    // Parse a bunch of declarators
    type_info.declarator = parse_declarator(&type_info, true);
    
    // TODO: Check validity in a better way
    if (type_info.identifier_token.contents == NULL) {
        // Error here
        parser_error_t error = {
            .invalid_token = t,
            .prev_token = t,
            .type = ERROR_UNEXPECTED_TOKEN,
            .offset = 0
        };
        report_error(error);

        return -1;
    }
    
    if (expect_token(T_LPAREN)) {
        return parse_function_definition(type_info);
    }
    else {
        
        return parse_declaration(type_info);
    }
    
    // If function parse 
}

static ast_node_h parse_translation_unit() {

    // A bunch of external_declarations
    
    ast_node_vector body = ast_node_vector_init(16);

    while(true) {
        ast_node_h decl = parse_toplevel_declaration();
        if (decl == -1) {
            break;
        }
        ast_node_vector_push(&(body), decl);
    }

    ast_node_h node = ast_program_init(body);

    return node;
}

void build_ast() {
    parser.ast_root = parse_translation_unit();
    end_parse();
    return;
}

void teardown_ast() {
    // Traverse the AST and free it.
    free_ast(parser.ast_root);
    return;
}

void init_parser(const char* source, uint32_t source_size) {
    parser.ast_root = -1;
    init_error_handler(source, source_size);
    init_binding_power();
    return;
}

ast_node_h get_root() {
    return parser.ast_root;
}
