#include "AST.h"
#include "AST_visitor.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

ast_node_t* init_ast_node() {
    ast_node_t* node = malloc(sizeof(*node));
    memset(node, 0, sizeof(ast_node_t));
    node->type = A_UNKNOWN;
    
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

