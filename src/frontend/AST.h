#ifndef AST_H
#define AST_H

#include "token.h"
#include "types.h"
#include "symbol_table.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define POSTORDER 1
#define PREORDER 0

#define PREFIX 1
#define POSTFIX 0

#define NEWSCOPE 1
#define OLDSCOPE 0

#define MAX_NUM_AST_NODES 255
#define MAX_SCOPE_RECURSION 8

typedef i16 ast_node_h;

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
#define T ast_node_h
#define NAME ast_node
#include "util/vector.h"
#undef T
#undef NAME

typedef struct AST_NODE_STRUCT {
    ast_node_enum type;
    u32 size; // Number of children including itself.
    token_t token;
    union { 
        // ==== Expressions: ====
        union {
            struct { // For int literal
                u32 value;
            } literal;
            struct {
                ast_node_h symbol_ref;
                ast_node_vector arguments; // expressions.
            } call;
            struct { // A reference to an already defined symbol
                type_info_t type;
                token_t token;
            } symbol;
            struct {
                ast_node_h left; 
                ast_node_h right;
            } assign; // Assign expr vs assign statement.
            struct { // For Operations or assign statements
                ast_op_enum type; 
                ast_node_h left;
                ast_node_h right;
            } binary;
            struct { // For Operations or assign statements
                ast_op_enum type;
                ast_node_h child;
                bool order; // post or pre
            } unary;
            struct { // For Operations or assign statements
                ast_op_enum type; 
                ast_node_h first;
                ast_node_h second;
                ast_node_h third;
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
                ast_node_h expression;
            } expression;
            struct {
                ast_node_h expression;
            } _return;
            struct {
                ast_node_h initilization;
                ast_node_h condition;
                ast_node_h update;
                ast_node_h body;
            } _for;
            struct {
                ast_node_h condition;
                ast_node_h body;
            } _while;
            struct {
                ast_node_h condition;
                ast_node_h if_stmt;
                ast_node_h else_stmt;
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
            ast_node_h initializer;
            type_info_t type_info;
            u32 scope;
            token_t token;
        } var_decl;
        struct {
            type_info_t type_info;
            u32 scope;
            token_t token;
        } param_decl;
        struct {
            ast_node_h body; // Compound statement
            type_info_t type_info;
            ast_node_vector parameters;
            u32 scope;
            token_t token;
        } func_decl; // Do the parameters need to have their own parm var decl node type?
        struct {
            ast_node_vector body;
            u32 main; // index into symbol table??
        } program;
    } as;
} ast_node_t;

// AST initialization:
ast_node_h ast_node_init(ast_node_enum type);
ast_node_h ast_int_literal_init(u32 value);
ast_node_h ast_expr_call_init(ast_node_h symbol_ref, ast_node_vector arguments);
ast_node_h ast_expr_symbol_init(token_t token);
ast_node_h ast_assign_expr_init(ast_node_h left, ast_node_h right);
ast_node_h ast_unary_op_init(ast_op_enum type, ast_node_h child, bool order);
ast_node_h ast_binary_op_init(ast_op_enum type, ast_node_h left, ast_node_h right);
ast_node_h ast_ternary_op_init(ast_op_enum type, ast_node_h left, ast_node_h right);
ast_node_h ast_compound_stmt_init(ast_node_vector statements, bool scope_flag);
ast_node_h ast_while_stmt_init(ast_node_h condition, ast_node_h body);
ast_node_h ast_for_stmt_init(ast_node_h intializer, ast_node_h condition, ast_node_h update, ast_node_h body);
ast_node_h ast_return_stmt_init(ast_node_h expression);
ast_node_h ast_if_stmt_init(ast_node_h condition, ast_node_h if_stmt, ast_node_h else_stmt);
ast_node_h ast_var_decl_init(ast_node_h initializer, type_info_t type_info, token_t token);
ast_node_h ast_param_decl_init(type_info_t type_info, token_t token);
ast_node_h ast_func_decl_init(ast_node_h body, ast_node_vector parameters, type_info_t type_info, token_t token);
ast_node_h ast_program_init(ast_node_vector body);
ast_node_h ast_inline_asm_init(token_t token);

// Takes a token type and returns the corresponding OP type.
typedef struct AST_VISITOR {
    enum {
        PRINT_AST,
        FREE_AST,
        CHECK_AST,
        ANALYSIS,
    } visitor_type;
    bool traversal_type; // POSTORDER or PREORDER
    union {
        struct {
            u32 indentation;
        } print_ast;
        struct {
            ast_node_enum* results;
            u32 index;
        } check_ast;
        struct {
            int useless;
        } analysis;
    } as;
} ast_visitor_t;

// Visitor options:
void print_ast(ast_node_h root);

void analyze_ast(ast_node_h root);

void check_ast(ast_node_h root, ast_node_enum* results);

void print_ast_node(ast_node_h node, u32 indentation);

void free_ast_node(ast_node_h node);

ast_node_t ast_node_data(ast_node_h node);

#endif
