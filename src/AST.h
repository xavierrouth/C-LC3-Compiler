#ifndef AST_H
#define AST_H

#include "type_table.h"

#include <stdlib.h>

typedef enum AST_OP_ENUM {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_NOT,
    OP_DIV,
    OP_MOD,
} ast_op_enum;

typedef enum AST_NODE_ENUM {
    A_PROGRAM,
    A_VAR_DECL,
    A_FUNCTION_CALL,
    A_FUNCTION_DECL,
    A_IDENTIFIER,
    A_COMPOUND_STMT,
    A_EXPR_STMT,
    A_VAR_EXPR,
    A_BINOP_EXPR,
    A_UNOP_EXPR,
    A_ASSIGN_EXPR,
    A_INTEGER_LITERAL,
} ast_node_enum;

typedef struct AST_NODE_STRUCT {
    ast_node_enum type;
    union {
        int lit_value; // For int literal
        struct { // For Variable declarations
            int id;
            int scope;
            type_enum type;
        } var;
        struct { // For Operations or assign statements
            ast_op_enum type; 
            struct AST_NODE_STRUCT* left;
            struct AST_NODE_STRUCT* right;
        } op;
        struct {
            int size;
            int capacity;
            struct AST_NODE_STRUCT** statements;
        } cmpd_stmt;
        struct {
            struct AST_NODE_STRUCT* expression;
        } expr_stmt;
        struct {
            struct AST_NODE_STRUCT* expression;
        } ret_stmt;
        struct {
            struct AST_NODE_STRUCT* expressions[3];
        } for_stmt;
        struct {
            struct AST_NODE_STRUCT* condition;
            struct AST_NODE_STRUCT* if_stmt;
            struct AST_NODE_STRUCT* else_stmt;
        } if_stmt;
        struct {
            struct AST_NODE_STRUCT* variable;
        } decl_stmt;
        struct {
            struct AST_NODE_STRUCT* cmpd;
            int main;
        } program;
         
    };
} ast_node_t;

ast_node_t* init_ast_node();

ast_node_t* init_compound_statement();

int compound_statement_push(ast_node_t* n, ast_node_t* statement);


#endif