#ifndef AST_H
#define AST_H

#include "token.h"
#include "types.h"
#include "symbol_table.h"

#include <stdlib.h>
#include <stdbool.h>

#define POSTORDER 1
#define PREORDER 0

#define PREFIX 1
#define POSTFIX 0

// Both unary and binary operators.
typedef enum AST_OP_ENUM {
    OP_INVALID = -1,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_NOT,
    OP_BITNOT,
    OP_INC,
    OP_DEC,
    OP_COMMA, // Wtf is the comma operator??
    // Keep prefix operators down here:
} ast_op_enum;

typedef enum AST_NODE_ENUM {
    A_UNKNOWN,
    A_PROGRAM,
    A_VAR_DECL,
    A_FUNCTION_CALL,
    A_FUNCTION_DECL,
    A_COMPOUND_STMT,
    A_EXPR_STMT,
    A_SYMBOL_REF,
    A_BINOP_EXPR,
    A_UNOP_EXPR,
    A_ASSIGN_EXPR,
    A_INTEGER_LITERAL,
    A_RETURN_STMT,
} ast_node_enum;

typedef struct AST_NODE_STRUCT ast_node_t;

#define T ast_node_t*
#define NAME ast_node
#include "vector.h"
#undef T
#undef NAME

/*
typedef struct ast_node_vector {
    ast_node_t** nodes;
    int size;
    int capacity;
    size_t elem_size; 
} ast_node_vector;
**/

//TODO: Move all char identifier[16]s to the symbol ref type.
// This way we can do scope correctly
struct AST_NODE_STRUCT {
    ast_node_enum type;
    int index; // Just some labeling.
    int size; // Number of children including itself.
    union { 
        struct { // For int literal
            int value;
        } literal;
        // Expressions:
        struct {
            ast_node_t* symbol_ref;
            ast_node_vector arguments; 
        } func_call;
        struct { // A reference to an already defined symbol
            int scope;
            type_enum type;
            char* identifier;
        } symbol_ref;
        struct {
            ast_node_t* left; 
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
            bool order; // post or pre
        } unary_op;
        struct { // For Operations or assign statements
            ast_op_enum type; 
            ast_node_t* first;
            ast_node_t* second;
            ast_node_t* third;
        } ternary_op;
        struct { // This is needed for scoping
            ast_node_vector statements;
            bool new_scope;
        } compound_stmt;
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
            ast_node_vector declarations;
        } decl_stmt;
        struct {
            char* identifier;
            ast_node_t* initializer;
            type_info_t type_info;
            int scope;
            bool is_parameter;
        } var_decl;
        struct {
            char* identifier;
            type_info_t type_info;
            int scope;
        } param_decl;
        struct {
            char* identifier;
            ast_node_t* body; // Compound statement
            type_info_t type_info;
            ast_node_vector parameters; // This should be a bunch of variable decls.
            int scope;
        } func_decl; // Do the parameters need to have their own parm var decl node type?
        struct {
            ast_node_vector body;
            int main; // index into symbol table??
        } program;
    } as;
};

// TODO: Should visitors pass a refernece to 
// themselves in the function that they are calling?
// Then they can edit stuff yknow.
// Well only ones that need to have to actually.

//TODO: Put the AST in the same place for all these function names, either beginning or end.
// TODO: Allow common initializer to init an ast node either on the stack or on the heap.
ast_node_t* ast_node_init();

//ast_node_t* create_ast_node();



// Takes a token type and returns the corresponding OP type.
ast_op_enum token_to_op(token_enum type);

#include "codegen.h"

typedef struct AST_NODE_VISITOR {
    enum {
        PRINT_AST,
        FREE_AST,
        CHECK_AST,
    } visitor_type;
    bool traversal_type; // POSTORDER or PREORDER
    union {
        struct {
            void (*func)(ast_node_t* node); // free_ast
        } free_ast;
        struct {
            void (*func)(ast_node_t* node, int indentation);
            int indentation;
        } print_ast;
        struct {
            ast_node_enum* results;
            int index;
        } check_ast;
    } as;
} ast_node_visitor;

// Visitor options:

void print_ast_node(ast_node_t* node, int indentation);

void print_ast(ast_node_t* root);

void free_ast_node(ast_node_t* node);

void free_ast(ast_node_t* root);

void ast_traversal(ast_node_t* root, ast_node_visitor* visitor);

// No func for node as it is a one liner.
void check_ast(ast_node_t* root, ast_node_enum* results);

#endif
