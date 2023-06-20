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

// This frees the list, this does not free the objects in the list.
void ast_node_list_free(ast_node_list list) {
    free(list.nodes);
    return;
}

// Use this same buffer for all the thingies
static char print_buffer[64];

static const char* ast_type_to_str(ast_node_enum type) {
    switch(type) {
        case A_ASSIGN_EXPR: return "A_ASSIGN_EXPR";
        case A_BINOP_EXPR: return "A_BINOP_EXPR";
        case A_COMPOUND_STMT: return "A_COMPOUND_STMT";
        case A_EXPR_STMT: return "A_EXPR_STMT";
        case A_PROGRAM: return "A_PROGRAM";
        case A_VAR_DECL: return "A_VARL_DECL";
        case A_INTEGER_LITERAL: return "A_INTEGER_LITERAL";
        case A_VAR_EXPR: return "A_VAR_EXPR";
    }
    // Should probably clear the buffer lol.
    memset(print_buffer, 0, 64);
    snprintf(print_buffer, 64, "%d", type);
    return print_buffer;
}

static const char* ast_op_to_str(ast_node_enum type) {
    switch(type) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_NOT: return "!";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
    }
    snprintf(print_buffer, 64, "%d", type);
    return print_buffer;
}


void print_ast_node(ast_node_t* node, int indentation) {
    switch (node->type) {
        case A_PROGRAM: {
            snprintf(print_buffer, 64, 
                "<type=%s>\n", \
                ast_type_to_str(node->type));
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_VAR_DECL: {
            snprintf(print_buffer, 64,
                "<type=%s, identifier=\"%s\">\n", \
                ast_type_to_str(node->type), 
                node->as.var_decl.identifier);   
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_VAR_EXPR: {
           snprintf(print_buffer, 64,
                "<type=%s, identifier=\"%s\">\n", \
                ast_type_to_str(node->type), 
                node->as.var_ref_expr.identifier);   
            printf_indent(indentation*3, print_buffer);
            return;    
        }
        case A_INTEGER_LITERAL: {
            snprintf(print_buffer, 64,
                "<type=%s, value=\"%d\">\n", \
                ast_type_to_str(node->type), 
                node->as.literal.value);
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_FUNCTION_DECL: {
            snprintf(print_buffer, 64,
                "<type=%s, identifier=\"%s\">\n", \
                ast_type_to_str(node->type), 
                node->as.fun_decl.identifier);
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_BINOP_EXPR: {
            snprintf(print_buffer, 64,
                "<type=%s, op_type=\"%s\">\n", \
                ast_type_to_str(node->type), 
                ast_op_to_str(node->as.binary_op.type));
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

void print_ast(ast_node_t* root, int indentation) {
    /**Pre-Order*/
    // This is just a pre-order traverssal that calls the print node function
    if (root == NULL)
        return;
    switch(root->type) {
        // Nodes with children
        case A_PROGRAM: {
            print_ast_node(root, indentation);
            for (int i = 0; i < (root->as.program.body.size); i++)
                print_ast(root->as.program.body.nodes[i], indentation + 1);
            return;
        }
        case A_VAR_DECL: {
            print_ast_node(root, indentation);
            // Check for NULL initializer
            print_ast(root->as.var_decl.initializer, indentation + 1);
            return;
        }
        // Expressions:
        case A_BINOP_EXPR: {
            print_ast_node(root, indentation);
            print_ast(root->as.binary_op.left, indentation + 1);
            print_ast(root->as.binary_op.right, indentation + 1);
            return;
        }
        // Terminal Nodes:
        case A_INTEGER_LITERAL: 
        case A_VAR_EXPR: 
            print_ast_node(root, indentation);
            return;
        default:
            printf("ERWTHJWK\n");
            return;
    }
    return;
}


void free_ast(ast_node_t* root) {
    /**Post-Order*/
    // This is just a pre-order traverssal that calls the print node function
    if (root == NULL)
        return;
    switch(root->type) {
        // Nodes with children
        case A_PROGRAM: {
            for (int i = 0; i < (root->as.program.body.size); i++)
                free_ast(root->as.program.body.nodes[i]);
            ast_node_list_free(root->as.program.body);
            free(root);
            return;
        }
        case A_VAR_DECL: {
            free_ast(root->as.var_decl.initializer);
            free(root);
            return;
        }
        // Expressions:
        case A_BINOP_EXPR: {
            free_ast(root->as.binary_op.left);
            free_ast(root->as.binary_op.right);
            free(root);
            return;
        }
        // Terminal Nodes:
        case A_INTEGER_LITERAL:
        case A_VAR_EXPR: 
            free(root);
            return;
        default:
            printf("ERWTHJWK\n");
            return;
    }
    return;
}