#ifndef CODEGEN_H
#define CODEGEN_H

#include "frontend/AST.h"

typedef struct {
    bool mul : 1;
    bool div : 1;
    bool mod : 1;
    bool rshift : 1;
} subroutines_t; 

typedef struct CODEGEN_STATE_STRUCT {
    bool regfile[8];

    uint16_t global_data_addr;
    uint16_t user_stack_addr;
    
} codegen_state_t;


void emit_ast(ast_node_t root);

void emit_ast_node(ast_node_t root);




#endif
