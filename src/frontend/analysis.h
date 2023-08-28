#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <stdbool.h>

#include "AST.h"
#include "types.h"

typedef struct ANALYSIS_STRUCT {
    struct {
        u8 idx;
        u8 scope_stack[MAX_SCOPE_RECURSION];
        i16 symbol_refs[MAX_NUM_AST_NODES]; // Maps symbol references to the scope it is used in
        i16 var_decls[MAX_NUM_AST_NODES]; // Maps variable declarations to the scope it occurs in
    } scopes;
    i16 offsets[MAX_NUM_AST_NODES];
    symbol_table_t symbol_table;
} analysis_t;

extern analysis_t analysis;

void analyze_ast_node(ast_node_h root);
void analysis_exit_ast_node(ast_node_h root);

void get_scope(ast_node_h ptr);

#endif
