#include "AST.h"
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

void free_compound_statement(ast_node_t* n) {
    free(n->cmpd_stmt.statements);
}

ast_node_t* init_compound_statement() {
    ast_node_t* n = init_ast_node();
    n->type = A_COMPOUND_STMT;
    const int initial_cap = 4;
    n->cmpd_stmt.size = 0;
    n->cmpd_stmt.capacity = initial_cap;
    n->cmpd_stmt.statements = malloc(sizeof(ast_node_t*) * initial_cap);
    return n;
}

int compound_statement_push(ast_node_t* n, ast_node_t* statement) {
    if (n->type != A_COMPOUND_STMT) {
        printf("Error");
        return -1;
    }
    
    if (n->cmpd_stmt.size >= n->cmpd_stmt.capacity) {
        n->cmpd_stmt.capacity *= 2; // Max this out;
        int cap = n->cmpd_stmt.capacity;
        n->cmpd_stmt.statements = realloc(n->cmpd_stmt.statements, sizeof(ast_node_t*) * cap);
    }

    n->cmpd_stmt.statements[n->cmpd_stmt.size++] = statement;
    return 0;
}