#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include "AST.h"
#include "codegen.h"

typedef struct AST_NODE_VISITOR {
    enum {
        PRINT_AST,
        FREE_AST,
        CHECK_AST,
        EMIT_AST,
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
        struct {
            void (*func)(ast_node_t* node, codegen_state_t* const state); // pointer to allow modification.
            codegen_state_t state;
        } emit_ast;
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