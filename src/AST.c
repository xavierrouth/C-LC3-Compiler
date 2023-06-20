#include "AST.h"
#include "util.h"
#include <stdio.h>

ast_node_t* init_ast_node() {
    ast_node_t* node = malloc(sizeof(*node));
    return node;
}

void free_ast_node(ast_node_t* n) {
    // Depends on type 
    free(n);
    return;
}

void visit_ast_node() {
    // Return a list of children depending on the type?
    return;
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

static char buffer[4];

static const char * ast_type_to_str(ast_node_enum type) {
    switch(type) {
        case A_ASSIGN_EXPR: return "A_ASSIGN_EXPR";
        case A_BINOP_EXPR: return "A_BINOP_EXPR";
        case A_COMPOUND_STMT: return "A_COMPOUND_STMT";
        case A_EXPR_STMT: return "A_EXPR_STMT";
        case A_PROGRAM: return "A_PROGRAM";
        case A_VAR_DECL: return "A_VARL_DECL";
        case A_INTEGER_LITERAL: return "A_INTEGER_LITERAL";
    }

    snprintf(buffer, 4, "%d", type);
    return buffer;

}

// Max size of tree print buffer
static char print_buffer[32];

void print_ast_node(ast_node_t* node, int indentation, int print_type) {
    switch (node->type) {
        case A_PROGRAM: {
            if (sprintf(print_buffer, "<type=%s, contents=\"%s\">\n", \
                ast_type_to_str(node->type), 
                "No contents yet."))
                printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_VAR_DECL: {
            if (sprintf(print_buffer, "<type=%s, contents=\"%s\">\n", \
                ast_type_to_str(node->type), 
                node->as.var_decl.identifier))   
                printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_INTEGER_LITERAL: {
            if (sprintf(print_buffer, "<type=%s, contents=\"%d\">\n", \
                ast_type_to_str(node->type), 
                node->as.literal.value))
                printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_FUNCTION_DECL: {
            if (sprintf(print_buffer, "<type=%s, contents=\"%s\">\n", \
                ast_type_to_str(node->type), 
                "No contents yet."))
                printf_indent(indentation*3, print_buffer);
            return;            
        }
        default:
           if (sprintf(print_buffer, "<type=%s, contents=\"%s\">\n", \
                ast_type_to_str(node->type), 
                "No contents yet."))
                printf_indent(indentation*3, print_buffer);
            return;  

    }
}