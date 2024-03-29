#ifndef AST_H
#define AST_H

#include "token.h"
#include "types.h"
#include "symbol_table.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// Handle for ast_node_t;
typedef int32_t ast_node_t; 

#define POSTORDER 1
#define PREORDER 0

#define PREFIX 1
#define POSTFIX 0

#define NEWSCOPE 1
#define OLDSCOPE 0

// Both unary and binary operators.
typedef enum AST_OP_ENUM {
    OP_INVALID = -1,
    // ==== Operators ====
    OP_ASSIGN, // = 

    OP_ADD, // '+'
    OP_SUB, // '-'
    OP_MUL, // '*'
    OP_DIV, // '/'
    OP_MOD, // '%'

    OP_LOGAND, // '&&'
    OP_LOGOR, // '||'
    OP_LOGNOT, // '!'

    OP_INCREMENT, // '++'
    OP_DECREMENT, // '--'

    // Binary Operators
    OP_BITAND, // '&'
    OP_BITOR, // '|'
    OP_BITXOR, // '^'
    OP_BITFLIP, // '~'
    OP_LEFTSHIFT, // '<<'
    OP_RIGHTSHIFT, // '>>'

    // Boolean Operators
    OP_LT, // '<'
    OP_GT, // '>'
    OP_LT_EQUAL, // '<='
    OP_GT_EQUAL, // '>='
    OP_NOTEQUALS, // '!='
    OP_EQUALS, // '==' 
    
    OP_DOT, // '.'
    OP_ARROW, // '->'
    OP_TERNARY, // '?'

    // Assignment operators
    OP_ASSIGN_LSHIFT, // <<=
    OP_ASSIGN_RSHIFT, // >>=
    OP_ASSIGN_MUL, // *=
    OP_ASSIGN_ADD, // '+='
    OP_ASSIGN_SUB, // '-='
    OP_ASSIGN_DIV, // '/='
    OP_ASSIGN_MOD, // '%='
    OP_ASSIGN_BITAND, // '&='
    OP_ASSIGN_BITXOR, // '^='
    OP_ASSIGN_BITOR, // '|='

    // Punctuation
    OP_CALL,
    OP_LPAREN, // '('
    OP_RPAREN, // ')'
    OP_LBRACKET, // '['
    OP_RBRACKET, // ']'

    OP_COLON, // ':'
    OP_COMMA, // ','
    OP_SEMICOLON, // ';'
    
} ast_op_enum;

typedef enum AST_NODE_ENUM {
    A_UNKNOWN,
    A_PROGRAM,
    // Statements:
    A_VAR_DECL,
    A_PARAM_DECL,
    A_FUNCTION_CALL,
    A_FUNCTION_DECL,
    A_COMPOUND_STMT,
    A_RETURN_STMT,
    A_IF_STMT,
    A_INLINE_ASM,
    // LOOPS:
    A_WHILE_STMT,
    A_FOR_STMT,
    
    // Expressions:
    A_SYMBOL_REF,
    A_TERNARY_EXPR,
    A_BINARY_EXPR,
    A_UNARY_EXPR,
    A_ASSIGN_EXPR,
    A_INTEGER_LITERAL,
} ast_node_enum;

// This is just a vector of handles.
#define T ast_node_t
#define NAME ast_node
#include "util/vector.h"
#undef T
#undef NAME

struct AST_NODE_STRUCT {
    ast_node_enum type;
    uint32_t size; // Number of children including itself.
    token_t token;
    union { 
        // ==== Expressions: ====
        union {
            struct { // For int literal
                uint32_t value;
            } literal;
            struct {
                ast_node_t symbol_ref;
                ast_node_vector arguments; // expressions.
            } call;
            struct { // A reference to an already defined symbol
                type_info_t type;
                token_t token;
            } symbol;
            struct {
                ast_node_t left; 
                ast_node_t right;
            } assign; // Assign expr vs assign statement.
            struct { // For Operations or assign statements
                ast_op_enum type; 
                ast_node_t left;
                ast_node_t right;
            } binary;
            struct { // For Operations or assign statements
                ast_op_enum type;
                ast_node_t child;
                bool order; // post or pre
            } unary;
            struct { // For Operations or assign statements
                ast_op_enum type; 
                ast_node_t first;
                ast_node_t second;
                ast_node_t third;
            } ternary;
        } expr;
        // ==== Statements: ====
        union {
            struct { 
                // Scope information here???
                ast_node_vector statements;
                bool scope_flag;
            } compound;
            struct {
                ast_node_t expression;
            } expression;
            struct {
                ast_node_t expression;
            } _return;
            struct {
                ast_node_t initilization;
                ast_node_t condition;
                ast_node_t update;
                ast_node_t body;
            } _for;
            struct {
                ast_node_t condition;
                ast_node_t body;
            } _while;
            struct {
                ast_node_t condition;
                ast_node_t if_stmt;
                ast_node_t else_stmt;
            } _if;
            struct {
                ast_node_vector declarations;
            } decl;
            struct {
                token_t token;
            } inline_asm;
        } stmt;
        // ==== Declarations: ====
        struct {
            ast_node_t initializer;
            type_info_t type_info;
            uint32_t scope;
            token_t token;
        } var_decl;
        struct {
            type_info_t type_info;
            uint32_t scope;
            token_t token;
        } param_decl;
        struct {
            ast_node_t body; // Compound statement
            type_info_t type_info;
            ast_node_vector parameters;
            uint32_t scope;
            token_t token;
        } func_decl; // Do the parameters need to have their own parm var decl node type?
        struct {
            ast_node_vector body;
            uint32_t main; // index into symbol table??
        } program;
    } as;
};

// AST initialization:
ast_node_t ast_node_init(ast_node_enum type);
ast_node_t ast_int_literal_init(uint32_t value);
ast_node_t ast_expr_call_init(ast_node_t symbol_ref, ast_node_vector arguments);
ast_node_t ast_expr_symbol_init(token_t token);
ast_node_t ast_assign_expr_init(ast_node_t left, ast_node_t right);
ast_node_t ast_unary_op_init(ast_op_enum type, ast_node_t child, bool order);
ast_node_t ast_binary_op_init(ast_op_enum type, ast_node_t left, ast_node_t right);
ast_node_t ast_ternary_op_init(ast_op_enum type, ast_node_t left, ast_node_t right);
ast_node_t ast_compound_stmt_init(ast_node_vector statements, bool scope_flag);
ast_node_t ast_while_stmt_init(ast_node_t condition, ast_node_t body);
ast_node_t ast_for_stmt_init(ast_node_t intializer, ast_node_t condition, ast_node_t update, ast_node_t body);
ast_node_t ast_return_stmt_init(ast_node_t expression);
ast_node_t ast_if_stmt_init(ast_node_t condition, ast_node_t if_stmt, ast_node_t else_stmt);
ast_node_t ast_var_decl_init(ast_node_t initializer, type_info_t type_info, token_t token);
ast_node_t ast_param_decl_init(type_info_t type_info, token_t token);
ast_node_t ast_func_decl_init(ast_node_t body, ast_node_vector parameters, type_info_t type_info, token_t token);
ast_node_t ast_program_init(ast_node_vector body);
ast_node_t ast_inline_asm_init(token_t token);
// Takes a token type and returns the corresponding OP type.
ast_op_enum token_to_op(token_enum type);

typedef struct AST_NODE_VISITOR {
    enum {
        PRINT_AST,
        FREE_AST,
        CHECK_AST,
        ANALYSIS,
    } visitor_type;
    bool traversal_type; // POSTORDER or PREORDER
    union {
        struct {
            uint32_t indentation;
        } print_ast;
        struct {
            ast_node_enum* results;
            uint32_t index;
        } check_ast;
        struct {
            int useless;
        } analysis;
    } as;
} ast_node_visitor;

// Visitor options:

void print_ast(ast_node_t root);

void analysis(ast_node_t root);

void check_ast(ast_node_t root, ast_node_enum* results);

void print_ast_node(ast_node_t node, uint32_t indentation);

void free_ast_node(ast_node_t node);

struct AST_NODE_STRUCT ast_node_data(ast_node_t node);

#endif
