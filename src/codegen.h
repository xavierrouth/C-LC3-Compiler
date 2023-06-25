#ifndef CODEGEN_H
#define CODEGEN_H

#include "AST.h"

typedef struct CODEGEN_STATE_STRUCT {
    bool regfile[8];
    int regfile_idx;
} codegen_state_t;

void emit_ast(ast_node_t* root);

void emit_ast_node(ast_node_t* root);




#endif
