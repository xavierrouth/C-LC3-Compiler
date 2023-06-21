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
    A_COMPOUND_STMT,
    A_EXPR_STMT,
    A_VAR_EXPR,
    A_BINOP_EXPR,
    A_UNOP_EXPR,
    A_ASSIGN_EXPR,
    A_INTEGER_LITERAL,
} ast_node_enum;

typedef struct AST_NODE_STRUCT ast_node_t;

typedef struct AST_NODE_LIST {
    ast_node_t** nodes;
    int size;
    int capacity;
    size_t elem_size; 
} ast_node_list;

//TODO: Move all char identifier[16]s to the symbol ref type.
// This way we can do scope correctly
struct AST_NODE_STRUCT {
    ast_node_enum type;
    union { 
        struct { // For int literal
            int value;
        } literal;
        struct {
            char identifier[16];
            ast_node_list parameters; // A bunch of expressions.
        } func_call_expr;
        struct {
            char identifier[16];
            int scope;
            // We don't know the type until we compare with var decl later.
            // We can kkeep track of scope for now though.
        } var_ref_expr;
        // Expressions:
        /**
        struct {
            ast_node_t* symbol;
            ast_node_list parameters; 
        } func_call;
        // TODO: Include symbol ref??
        struct { // A reference to an already defined symbol
            int id;
            int scope;
            type_enum type;
        } symbol_ref;
        */
        struct {
            //ast_node_t* left; // This has to be an indeitifer
            char identifier[16];
            ast_node_t* right;
        } assign_expr; // Assign expr vs assign statement.
        struct { // For Operations or assign statements
            ast_op_enum type; 
            ast_node_t* left;
            ast_node_t* right;
        } binary_op;
        struct { // For Operations or assign statements
            ast_op_enum type; 
            ast_node_t* child;
        } unary_op;
        struct { // For Operations or assign statements
            ast_op_enum type; 
            ast_node_t* first;
            ast_node_t* second;
            ast_node_t* third;
        } ternary_op;
        struct { // Dynamic list of statements
            ast_node_list statements;
        } commpound_stmt;
        struct {
            ast_node_t* expression;
        } expression_stmt;
        struct {
            ast_node_t* expression;
        } return_stmt;
        struct {
            ast_node_t* initilization;
            ast_node_t* condition;
            ast_node_t* update;
        } for_stmt;
        struct {
            ast_node_t* condition;
            ast_node_t* if_stmt;
            ast_node_t* else_stmt;
        } if_stmt;
        // Do we need to have a decl_stmt node 
        struct {
            ast_node_list declarations;
        } decl_stmt;
        struct {
            char identifier[16];
            ast_node_t* initializer;
            type_info_t type_info;
        } var_decl;
        struct {
            char identifier[16];
            ast_node_list body;
            type_info_t type_info;
            // TODO: Parameters
        } func_decl;
        struct {
            ast_node_list body;
            int main; // index into symbol table??
        } program;
    } as;
};
 
ast_node_t* init_ast_node();

ast_node_list ast_node_list_init();

void ast_node_list_push(ast_node_list* list, ast_node_t* node);

void print_ast_node(ast_node_t* node, int indentation);

void print_ast(ast_node_t* root, int indentation);

void free_ast(ast_node_t* root);

#endif