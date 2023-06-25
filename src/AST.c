#include "AST.h"
#include "token.h"
#include "AST_visitor.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

ast_op_enum token_to_op(token_enum type) {
    switch (type) {
        case T_SUB: return OP_SUB;
        case T_ADD: return OP_ADD;
        case T_MUL: return OP_MUL;
        case T_COMMA: return OP_COMMA;
        //case T_NOT: return OP_NOT;
        case T_DIV: return OP_DIV;
        default: return OP_INVALID;
    }
    
}

// Sometimes we init and then free, so we will jump over some labels.
// Oh well!
ast_node_t* ast_node_init() {
    static int node_labels = 0;
    ast_node_t* node = malloc(sizeof(*node));
    memset(node, 0, sizeof(ast_node_t));
    node->type = A_UNKNOWN;
    node->size = 1;
    node->index = node_labels++; 
    return node;
}

ast_node_list ast_node_list_init() {
    ast_node_list list;
    list.capacity = 4;
    list.size = 0;
    list.elem_size = sizeof(ast_node_t*);
    list.nodes = malloc(list.elem_size * list.capacity);
    return list;
}

void ast_node_list_push(ast_node_list* list, ast_node_t* node) {
    if (list->size >= list->capacity) {
        list->capacity *= 2;
        list->nodes = realloc(list->nodes, list->elem_size * list->capacity);
    }
    list->nodes[list->size++] = node;
}

// This frees the list, this does not free the objects in the list.
void ast_node_list_free(ast_node_list list) {
    free(list.nodes);
    return;
}

// TOOD: AST Node initializers
ast_node_t* ast_return_stmt_init(ast_node_t* expression) {
    ast_node_t* node = ast_node_init();
    node->type = A_RETURN_STMT;
    node->as.return_stmt.expression = expression;
    return node;
}

ast_node_t* ast_program_init(ast_node_t* main, ast_node_list body) {
    ast_node_t* node = ast_node_init();
    node->type = A_PROGRAM;
    node->as.program.body = body;
    node->as.program.main = main;
    return node;
}

ast_node_t* ast_func_decl_init(ast_node_t* body, ast_node_list parameters, type_info_t type_info, char* identifier) {
    return body;
}

