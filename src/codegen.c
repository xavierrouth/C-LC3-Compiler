#include <stdio.h>

#include "codegen.h"
#include "AST.h"

#include <stdarg.h>

// The whole visitor pattern is stupid.

// Register states
#define UNUSED 0
#define USED 1

static codegen_state_t state;

static FILE* File;

void set_out_file(char* path) {
    File = fopen(path, "w");
    return;
}

void close_out_file() {
    fclose(File);
}

static void emitf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(File, fmt, args);
    va_end(args);
    return;
}

// Just know for now that regfile is 8 registesr???/
static uint16_t get_empty_reg() {
    for (uint16_t i = 0; i < 8; i++) {
        if (state.regfile[i] == UNUSED) {
            return i;
        }
    }
    emitf("No empty registers, please write a shorter expression.\n");
    return -1; // Will j break I guess.
}

void emit_ast(ast_node_t root) {
    emit_ast_node(root);
    return;
} 

// Sethi-Ullman Register Allocation for the expression rooted at this node.
static int emit_expression_node(ast_node_t node_h) {
    // TODO: Add immediates.
    struct AST_NODE_STRUCT node = ast_node_data(node_h);

    if (node.type == A_BINARY_EXPR) {
        switch (node.as.expr.binary.type) {
            struct AST_NODE_STRUCT left = ast_node_data(node.as.expr.binary.left);
            struct AST_NODE_STRUCT right = ast_node_data(node.as.expr.binary.right);
            case OP_ADD: {
                int32_t r1 = 0;
                // we can only use one immediate though.
                if (left.type == A_INTEGER_LITERAL && left.as.expr.literal.value <= 15) {
                    // If the leaf is an int literal, we can just use an immediate.
                    r1 = emit_expression_node(node.as.expr.binary.right);
                    emitf("ADD R%d, R%d, #%d\n", r1, r1, left.as.expr.literal.value);
                    state.regfile[r1] = USED;
                    
                }
                else if (right.type == A_INTEGER_LITERAL && right.as.expr.literal.value <= 15) {
                    r1 = emit_expression_node(node.as.expr.binary.left);
                    int32_t val = right.as.expr.literal.value;
                    emitf("ADD R%d, R%d, #%d\n", r1, r1, val);
                    // TOOD: Only if the immediate is small enough
                    state.regfile[r1] = USED;
                }
                else {
                    r1 = emit_expression_node(node.as.expr.binary.left);
                    state.regfile[r1] = USED;
                    int32_t r2 = emit_expression_node(node.as.expr.binary.right);
                    state.regfile[r2] = UNUSED;
                    emitf("ADD R%d, R%d, R%d\n", r1, r1, r2);
                }
            return r1;
            }
            case OP_SUB: {
                int32_t r1 = 0;
                // we can only use one immediate though.
                // If the left side is literal, oh well we can't do anything about that???
                if (right.type == A_INTEGER_LITERAL) {
                    r1 = emit_expression_node(node.as.expr.binary.left);
                    int32_t val = -1 * right.as.expr.literal.value;
                    emitf("ADD R%d, R%d, #%d\n", r1, r1, val);
                    // TOOD: Only if the immediate is small enough
                    state.regfile[r1] = USED;
                }
                else {
                    // Otherwise the val is in a register somehow.
                    r1 = emit_expression_node(node.as.expr.binary.left);
                    state.regfile[r1] = USED;
                    int32_t r2 = emit_expression_node(node.as.expr.binary.right);
                    emitf("NOT R%d, R%d\n", r2, r2);
                    emitf("ADD R%d, R%d, #1\n", r2, r2);
                    emitf("ADD R%d, R%d, R%d\n", r1, r1, r2);
                    state.regfile[r2] = UNUSED;
                }
            return r1;
            }
        }
    }
    else if (node.type == A_UNARY_EXPR) {
        switch (node.as.expr.unary.type) {
            case OP_ADD: {
                // Litreally doens't do anything, just return 
                return emit_expression_node(node.as.expr.unary.child);
            }
            case OP_SUB: {
                // Get the register that the child expression is in.
                int r = emit_expression_node(node.as.expr.unary.child);
                // Negate it.
                emitf("NOT R%d, R%d\n", r, r);
                emitf("ADD R%d, R%d, #1\n", r, r);
                return r; // Its stored in the same register still.
            }
        }
    }
    else if (node.type == A_FUNCTION_CALL) {
        // Push arguments right to left.
        for (int i = node.as.expr.call.arguments.size - 1; i >= 0; i--) {
            int r = emit_expression_node(node.as.expr.call.arguments.data[i]);
            // Push r to stack, load next 
            emitf("ADD R6, R6, #-1\n");
            emitf("STR R%d, R6, #0 ; Push parameter to stack frame\n", r);
            emitf("\n");
        }
        emitf("JSR %s\n\n", ast_node_data(node.as.expr.call.symbol_ref).as.expr.symbol.identifier);
        int ret = get_empty_reg();
        // load 
        emitf("LDR R%d, R6, #0 ; Load the return value\n", ret);
        emitf("ADD R6, R6, #1 ; Pop return value\n");
        emitf("ADD R6, R6, #%d ; Pop arguments\n", node.as.expr.call.arguments.size);
        return ret;
    }
    else if (node.type == A_INTEGER_LITERAL) {
        // We probably just want to place this in a constant pool and load from there.
        // Get a register to place this in
        int val = node.as.expr.literal.value;
        // TODO: If val is > 15 then we have to materialize this int somehow.
        int r1 = get_empty_reg();
        emitf("AND R%d, R%d, x0000\n", r1, r1);
        if (val != 0) {
            emitf("ADD R%d, R%d, #%d\n", r1, r1, val);
        }
        state.regfile[r1] = USED;
        return r1;
    }
    else if (node.type == A_SYMBOL_REF) {
        // Do now
        symtable_entry* sym_data = symtable_search(node.as.expr.symbol.scope, node.as.expr.symbol.identifier);
        // Load 
        // Parameters need a positive offset??
        int offset = sym_data->offset;
        // Is a parameter
        int r1 = get_empty_reg();
        if (sym_data->type == PARAMETER) { // Is a parameter
            offset = sym_data->offset + 4; // These should be positive.
            emitf("LDR R%d, R5, #%d ; Load parameter \"%s\"\n", r1, offset, sym_data->identifier);
        }
        else { // Not a parameter
            emitf("LDR R%d, R5, #%d ; Load local variable \"%s\"\n", r1, offset, sym_data->identifier);
        }
        
        
        return r1;
    }
    
    
}

void emit_ast_node(ast_node_t node_h) {
    if (node_h == -1) {
        return;
    }

    struct AST_NODE_STRUCT node = ast_node_data(node_h);

    switch (node.type) {
        case A_PROGRAM: {
            emitf(".ORIG x3000\n");
            
            emitf("LD R6, USER_STACK\n"); // start the stack pointer at the end of user space
            emitf("JSR main\n");
            for (int i = 0; i < (node.as.program.body.size); i++)
                emit_ast_node(node.as.program.body.data[i]);
            emitf("HALT\n");
            emitf("USER_STACK .FILL xFDFF\n");
            emitf(".END\n");
            return;
        }
        case A_ASSIGN_EXPR: {
            // Needs to have access to the children.
            // Need to pass a reference of the prev results (child nodes) through the function call,
            // using 
            // This requires a symbol ref. We can't do those yet.
            // This means store.
            int reg = emit_expression_node(node.as.expr.assign.right);
            //TODO:
            symtable_entry* sym_data = NULL; // symtable_search(node.as.expr.assign.left->as.symbol_ref.scope, node.as.expr.assign.left->as.symbol_ref.identifier);
            
            emitf("STR R%d, R5, #%d ; Assign to variable\n\n", reg, sym_data->offset);
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
            int reg = emit_expression_node(node.as.stmt._return.expression);
            // Write it into return value slot, which 
            emitf("\n");
            emitf("STR R%d, R5, #3 ; Write return value, always R5 + 3\n", reg);
            emitf("\n");
            return;
        }
        case A_COMPOUND_STMT: {
            for (int i = 0; i < (node.as.stmt.compound.statements.size); i++)
                emit_ast_node(node.as.stmt.compound.statements.data[i]);
            break;
        }
        case A_FUNCTION_DECL: {
            state.regfile[0] = UNUSED;
            state.regfile[1] = UNUSED;
            state.regfile[2] = UNUSED;
            state.regfile[3] = UNUSED;
            // Callee save registers:
            emitf("\n");
            emitf("; Begin function %s:\n", node.as.func_decl.identifier);
            emitf("%s\n", node.as.func_decl.identifier);
            emitf("; Callee Setup: \n");
            emitf("ADD R6, R6, #-1 ; Allocate spot for the return value\n");
            emitf("\n");
            emitf("ADD R6, R6, #-1 ; \n");
            emitf("STR R7, R6, #0  ; Push R7 (Return Address)\n");
            emitf("\n");
            emitf("ADD R6, R6, #-1 ; \n");
            emitf("STR R5, R6, #0  ; Push R5 (Caller's Frame Pointer)\n");
            emitf("\n");
            emitf("ADD R5, R6, #-1 ; Set frame pointer for function\n");
            emitf("\n");
            emitf("; Perform work for function: \n");
            emit_ast_node(node.as.func_decl.body);
            emitf("; Callee Teardown: \n");
            emitf("ADD R6, R5, #1 ; Pop local variables\n");
            emitf("\n");
            emitf("LDR R5, R6, #0 ; Pop the frame pointer\n");
            emitf("ADD R6, R6, #1\n");
            emitf("\n");
            emitf("LDR R7, R6, #0 ; Pop the return address\n");
            emitf("ADD R6, R6, #1\n");
            emitf("RET\n");
            emitf("; End function %s\n", node.as.func_decl.identifier);
            emitf("\n");
            break;
        }
        case A_PARAM_DECL: 
            // Don't do anything.
            break;
        case A_VAR_DECL:
            // Is global scope
            // We will have a global data section, instead of placing them in the same order
            // as they are declared in the program.
            // Global data section is at address x5000
            // Global variable
            if (node.as.var_decl.scope == 0) {
                emitf("%s .FILL x0000\n", node.as.var_decl.identifier);
                // No initializer.
                return;
            }
            
            
            // Local Variable, 
            // allocate space for local variable
            emitf("ADD R6, R6, #-1 ; Allocate space for \"%s\"\n", node.as.var_decl.identifier);
            if (node.as.var_decl.initializer != NULL) {
                // TOOD: Assignment nodes should not really be separate, there should be an assignemnt expression.
                // But for now, we can kepe initialization separate, as for static ints it works different I suppose
                // Load a reg with the initializer value
                int reg = emit_expression_node(node.as.var_decl.initializer);
                symtable_entry* sym_data = symtable_search(node.as.var_decl.scope, node.as.var_decl.identifier);
                
                emitf("STR R%d, R5, #%d ; Initialize variable\n\n", reg, sym_data->offset);
                state.regfile[reg] = UNUSED;
            }
            return;
            

        case A_UNARY_EXPR:
        case A_BINARY_EXPR: 
        case A_INTEGER_LITERAL: 
        case A_SYMBOL_REF: 
            emit_expression_node(node_h);
            return;
        
    }
}