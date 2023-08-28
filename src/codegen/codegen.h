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

    u16 global_data_addr;
    u16 user_stack_addr;
    char* current_function_name;
    
} codegen_state_t;

void init_codegen();

void emit_ast(ast_node_h root);

void emit_ast_node(ast_node_h root);




#endif
