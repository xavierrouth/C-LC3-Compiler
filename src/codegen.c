#include <stdio.h>

#include "codegen.h"
#include "AST.h"

// The whole visitor pattern is stupid.

// Register states
#define UNUSED 0
#define USED 1

static codegen_state_t state;

// Just know for now that regfile is 8 registesr???/
static int get_empty_reg() {
    for (int i = 0; i < 8; i++) {
        if (state.regfile[i] == UNUSED) {
            return i;
        }
    }
    printf("No empty registers, please write a shorter expression.\n");
    return -1; // Will j break I guess.
}

void emit_ast(ast_node_t* root) {
    emit_ast_node(root);
    return;
} 

// Sethi-Ullman Register Allocation for the expression rooted at this node.
static int emit_expression_node(ast_node_t* node) {
    // TODO: Add immediates.
    if (node->type == A_BINOP_EXPR) {
        switch (node->as.binary_op.type) {
            case OP_ADD: {
                int r1 = 0;
                // we can only use one immediate though.
                if (node->as.binary_op.left->type == A_INTEGER_LITERAL && \
                    node->as.binary_op.left->as.literal.value <= 15) {
                    // If the leaf is an int literal, we can just use an immediate.
                    r1 = emit_expression_node(node->as.binary_op.right);
                    printf("ADD R%d, R%d, #%d\n", r1, r1, node->as.binary_op.left->as.literal.value);
                    state.regfile[r1] = USED;
                    
                }
                else if (node->as.binary_op.right->type == A_INTEGER_LITERAL && \
                    node->as.binary_op.right->as.literal.value <= 15) {
                    r1 = emit_expression_node(node->as.binary_op.left);
                    int val = node->as.binary_op.right->as.literal.value;
                    printf("ADD R%d, R%d, #%d\n", r1, r1, val);
                    // TOOD: Only if the immediate is small enough
                    state.regfile[r1] = USED;
                }
                else {
                    r1 = emit_expression_node(node->as.binary_op.left);
                    state.regfile[r1] = USED;
                    int r2 = emit_expression_node(node->as.binary_op.right);
                    state.regfile[r2] = UNUSED;
                    printf("ADD R%d, R%d, R%d\n", r1, r1, r2);
                }
            return r1;
            }
            case OP_SUB: {
                int r1 = 0;
                // we can only use one immediate though.
                // If the left side is literal, oh well we can't do anything about that???
                if (node->as.binary_op.right->type == A_INTEGER_LITERAL) {
                    r1 = emit_expression_node(node->as.binary_op.left);
                    int val = -1 * node->as.binary_op.right->as.literal.value;
                    printf("ADD R%d, R%d, #%d\n", r1, r1, val);
                    // TOOD: Only if the immediate is small enough
                    state.regfile[r1] = USED;
                }
                else {
                    // Otherwise the val is in a register somehow.
                    r1 = emit_expression_node(node->as.binary_op.left);
                    state.regfile[r1] = USED;
                    int r2 = emit_expression_node(node->as.binary_op.right);
                    printf("NOT R%d, R%d\n", r2, r2);
                    printf("ADD R%d, R%d, #1\n", r2, r2);
                    printf("ADD R%d, R%d, R%d\n", r1, r1, r2);
                    state.regfile[r2] = UNUSED;
                }
            return r1;
            }
        }
    }
    else if (node->type == A_UNOP_EXPR) {
        switch (node->as.unary_op.type) {
            case OP_ADD: {
                // Litreally doens't do anything, just return 
                return emit_expression_node(node->as.unary_op.child);
            }
            case OP_SUB: {
                // Get the register that the child expression is in.
                int r = emit_expression_node(node->as.unary_op.child);
                // Negate it.
                printf("NOT R%d, R%d\n", r, r);
                printf("ADD R%d, R%d, #1\n", r, r);
                return r; // Its stored in the same register still.
            }
        }
    }
    else if (node->type == A_FUNCTION_CALL) {
        node->as.func_call.symbol_ref;

        // Push arguments right to left.
        for (int i = node->as.func_call.arguments.size - 1; i >= 0; i--) {
            int r = emit_expression_node(node->as.func_call.arguments.data[i]);
            // Push r to stack, load next 
            printf("ADD R6, R6, #-1\n");
            printf("STR R%d, R6, #0 ; Push parameter to stack frame\n", r);
            printf("\n");
        }
        printf("JSR %s\n\n", node->as.func_call.symbol_ref->as.symbol_ref.identifier);
        int ret = get_empty_reg();
        // load 
        printf("LDR R%d, R6, #0 ; Load the return value\n", ret);
        printf("ADD R6, R6, #1 ; Pop return value\n");
        printf("ADD R6, R6, #%d ; Pop arguments\n", node->as.func_call.arguments.size);
        return ret;
    }
    else if (node->type == A_INTEGER_LITERAL) {
        // We probably just want to place this in a constant pool and load from there.
        // Get a register to place this in
        int val = node->as.literal.value;
        // TODO: If val is > 15 then we have to materialize this int somehow.
        int r1 = get_empty_reg();
        printf("AND R%d, R%d, x0000\n", r1, r1);
        if (val != 0) {
            printf("ADD R%d, R%d, #%d\n", r1, r1, val);
        }
        state.regfile[r1] = USED;
        return r1;
    }
    else if (node->type == A_SYMBOL_REF) {
        // Do now
        symbol_table_entry* sym_data = symbol_table_search(node->as.symbol_ref.scope, node->as.symbol_ref.identifier);
        // Load 
        // Parameters need a positive offset??
        int offset = sym_data->stack_offset;
        // Is a parameter
        int r1 = get_empty_reg();
        if (sym_data->parameter) { // Is a parameter
            offset = sym_data->stack_offset + 4; // These should be positive.
            printf("LDR R%d, R5, #%d ; Load parameter \"%s\"\n", r1, offset, sym_data->identifier);
        }
        else { // Not a parameter
            printf("LDR R%d, R5, #%d ; Load local variable \"%s\"\n", r1, offset, sym_data->identifier);
        }
        
        
        return r1;
    }
    
    
}

void emit_ast_node(ast_node_t* node) {
    if (node == NULL) {
        return;
    }
    switch (node->type) {
        case A_PROGRAM: {
            printf(".ORIG x3000\n");
            
            printf("LD R6, USER_STACK\n"); // start the stack pointer at the end of user space
            printf("JSR main\n");
            for (int i = 0; i < (node->as.program.body.size); i++)
                emit_ast_node(node->as.program.body.data[i]);
            printf("HALT\n");
            printf("USER_STACK .FILL xFDFF\n");
            printf(".END\n");
            return;
        }
        case A_ASSIGN_EXPR: {
            // Needs to have access to the children.
            // Need to pass a reference of the prev results (child nodes) through the function call,
            // using 
            // This requires a symbol ref. We can't do those yet.
            // This means store.
            int reg = emit_expression_node(node->as.assign_expr.right);
            symbol_table_entry* sym_data = symbol_table_search(node->as.assign_expr.left->as.symbol_ref.scope, node->as.assign_expr.left->as.symbol_ref.identifier);
            
            printf("STR R%d, R5, #%d ; Assign to variable\n\n", reg, sym_data->stack_offset);
            state.regfile[reg] = UNUSED;
            return;
        }
        // Somehow we need to pattern match these two into the same instruction.
        // Binops on immediates can be one instruction if the immediate is small enough.
        // How does one even do that?
        case A_RETURN_STMT: {
            // Load the child into R0
            // Check if its already in R0???
            // No optimizations yet.
            int reg = emit_expression_node(node->as.return_stmt.expression);
            // Write it into return value slot, which 
            printf("\n");
            printf("STR R%d, R5, #3 ; Write return value, always R5 + 3\n", reg);
            printf("\n");
            return;
        }
        case A_COMPOUND_STMT: {
            for (int i = 0; i < (node->as.commpound_stmt.statements.size); i++)
                emit_ast_node(node->as.commpound_stmt.statements.data[i]);
            break;
        }
        case A_FUNCTION_DECL: {
            state.regfile[0] = UNUSED;
            state.regfile[1] = UNUSED;
            state.regfile[2] = UNUSED;
            state.regfile[3] = UNUSED;
            // Callee save registers:
            printf("\n");
            printf("; Begin function %s:\n", node->as.func_decl.identifier);
            printf("%s\n", node->as.func_decl.identifier);
            printf("; Callee Setup: \n");
            printf("ADD R6, R6, #-1 ; Allocate spot for the return value\n");
            printf("\n");
            printf("ADD R6, R6, #-1 ; \n");
            printf("STR R7, R6, #0  ; Push R7 (Return Address)\n");
            printf("\n");
            printf("ADD R6, R6, #-1 ; \n");
            printf("STR R5, R6, #0  ; Push R5 (Caller's Frame Pointer)\n");
            printf("\n");
            printf("ADD R5, R6, #-1 ; Set frame pointer for function\n");
            printf("\n");
            printf("; Perform work for function: \n");
            emit_ast_node(node->as.func_decl.body);
            printf("; Callee Teardown: \n");
            printf("ADD R6, R5, #1 ; Pop local variables\n");
            printf("\n");
            printf("LDR R5, R6, #0 ; Pop the frame pointer\n");
            printf("ADD R6, R6, #1\n");
            printf("\n");
            printf("LDR R7, R6, #0 ; Pop the return address\n");
            printf("ADD R6, R6, #1\n");
            printf("RET\n");
            printf("; End function %s\n", node->as.func_decl.identifier);
            printf("\n");
            break;
        }
        case A_VAR_DECL:
            // Is global scope
            // We will have a global data section, instead of placing them in the same order
            // as they are declared in the program.
            // Global data section is at address x5000
            // Global variable
            if (node->as.var_decl.scope->parent == NULL) {
                printf("%s .FILL x0000\n", node->as.var_decl.identifier);
                // No initializer.
                return;
            }
            
            
            // Local Variable, but not a parameter.
            // Check current scope:
            if (!node->as.var_decl.is_parameter) {
                // allocate space for local variable
                printf("ADD R6, R6, #-1 ; Allocate space for \"%s\"\n", node->as.var_decl.identifier);
                if (node->as.var_decl.initializer != NULL) {
                    // TOOD: Assignment nodes should not really be separate, there should be an assignemnt expression.
                    // But for now, we can kepe initialization separate, as for static ints it works different I suppose
                    // Load a reg with the initializer value
                    int reg = emit_expression_node(node->as.var_decl.initializer);
                    symbol_table_entry* sym_data = symbol_table_search(node->as.var_decl.scope, node->as.var_decl.identifier);
                    
                    printf("STR R%d, R5, #%d ; Initialize variable\n\n", reg, sym_data->stack_offset);
                    state.regfile[reg] = UNUSED;
                }
                return;
            }
           
        case A_BINOP_EXPR: 
        case A_INTEGER_LITERAL: 
        case A_SYMBOL_REF: 
            emit_expression_node(node);
            return;
        
    }
}