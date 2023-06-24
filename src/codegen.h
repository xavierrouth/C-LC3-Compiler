#ifndef CODEGEN_H
#define CODEGEN_H

#include "AST.h"

typedef struct CODEGEN_STATE_STRUCT {
    int regfile[8];
    int regfile_idx;
    int ops[2]; // The registers that the operands to the current node are place in.
    int op_idx;
    // Need to store the state of the stack also. i.e. what variables are where
    // on the stack and so we can access them from FP and SP.
} codegen_state_t;

void emit_ast(ast_node_t* root);

void emit_ast_node(ast_node_t* root, codegen_state_t* const state);




#endif
