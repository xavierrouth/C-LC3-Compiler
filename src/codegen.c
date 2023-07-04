#include <stdio.h>
#include <stdarg.h>

#include "codegen.h"
#include "AST.h"
#include "symbol_table.h"

// The whole visitor pattern is stupid.

// Register states
#define UNUSED 0
#define USED 1

extern int32_t symbol_ref_scopes[];
extern int32_t var_decl_scopes[];

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

// TODO: Make this better:
static char* current_function_name;

static int emit_expression_node(ast_node_t node_h);

static uint32_t emit_condition_node(ast_node_t node_h) {
    struct AST_NODE_STRUCT node = ast_node_data(node_h);

    int32_t right = emit_expression_node(node.as.expr.binary.right);
    int32_t left = emit_expression_node(node.as.expr.binary.left);

    state.regfile[right] = UNUSED;
    state.regfile[left] = UNUSED;

    int32_t ret = left;
    int32_t scratch = right;

    // Store result in left always.
    switch (node.as.expr.binary.type) {
        case OP_LT: { // Sub left from right.
            emitf("NOT R%d, R%d ; Test '<' \n", left, left);
            emitf("ADD R%d, R%d, #1\n", left, left);
            emitf("ADD R%d, R%d, R%d\n", ret, left, right);
            break;
        }
        case OP_GT: {
            emitf("NOT R%d, R%d ; Test '>' \n", right, right);
            emitf("ADD R%d, R%d, #1\n", right, right);
            emitf("ADD R%d, R%d, R%d\n", ret, left, right);
            break;
        }
        case OP_EQUALS: {
            //TODO: Optimize this:
            
            emitf("NOT R%d, R%d ; Test '<' \n", left, left);
            emitf("ADD R%d, R%d, #1\n", left, left);
            emitf("ADD R%d, R%d, R%d\n", ret, left, right);
            emitf("BRz #3\n"); // If its zero, skip the next stepa
            emitf("AND R%d, R%d, #0\n", ret, ret); // Zero out reg 
            emitf("BRz #3\n");
            emitf("AND R%d, R%d, #0\n", ret, ret); // Need to put negative in this register
            emitf("ADD R%d, R%d, #1\n", ret, ret); // This needs to be positive
            break;
        }
        case OP_NOTEQUALS: {
            emitf("DONT DO NOTEQULAS");
            break;
        }

    }
    state.regfile[left] = USED;
    return left;
}

// Sethi-Ullman Register Allocation for the expression rooted at this node.
static int emit_expression_node(ast_node_t node_h) {
    // TODO: Add immediates.
    struct AST_NODE_STRUCT node = ast_node_data(node_h);

    if (node.type == A_BINARY_EXPR) {
        struct AST_NODE_STRUCT left = ast_node_data(node.as.expr.binary.left);
        struct AST_NODE_STRUCT right = ast_node_data(node.as.expr.binary.right);
        switch (node.as.expr.binary.type) {
            case OP_ASSIGN: {
                int32_t reg = emit_expression_node(node.as.expr.binary.right);

                //struct AST_NODE_STRUCT child = ast_node_data(node.as.)
                symbol_table_entry_t symbol = symbol_table_search(symbol_ref_scopes[node.as.expr.binary.left], left.as.expr.symbol.identifier);
            
                emitf("STR R%d, R5, #%d ; Assign to variable \"%s\"\n\n", reg, -1 * symbol.offset, symbol.identifier);
                state.regfile[reg] = UNUSED;
                return reg;
            }
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
            // Conditional Operators::
            case OP_EQUALS: 
            case OP_LT:
            case OP_GT:
            case OP_NOTEQUALS:
            case OP_GT_EQUAL:
            case OP_LT_EQUAL:
                return emit_condition_node(node_h);
            
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
            emitf("STR R%d, R6, #0 ; Push argument to stack frame\n", r);
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
        symbol_table_entry_t symbol = symbol_table_search(symbol_ref_scopes[node_h], node.as.expr.symbol.identifier);
        // Load 
        // Parameters need a positive offset??
        // Is a parameter
        int r1 = get_empty_reg();
        if (symbol.type == PARAMETER) { // Is a parameter
            emitf("LDR R%d, R5, #%d ; Load parameter \"%s\"\n", r1, symbol.offset + 4, symbol.identifier);
        }
        else { // Not a parameter
            emitf("LDR R%d, R5, #%d ; Load local variable \"%s\"\n", r1, -1 * symbol.offset, symbol.identifier);
        }
        return r1;
    }
}

void emit_ast_node(ast_node_t node_h) {
    static uint32_t if_counter = 0;
    if (node_h == -1) {
        return;
    }

    struct AST_NODE_STRUCT node = ast_node_data(node_h);

    switch (node.type) {
        state.regfile[0] = UNUSED;
        state.regfile[1] = UNUSED;
        state.regfile[2] = UNUSED;
        state.regfile[3] = UNUSED;
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
        // Somehow we need to pattern match these two into the same instruction.
        // Binops on immediates can be one instruction if the immediate is small enough.
        // How does one even do that?
        case A_RETURN_STMT: {
            state.regfile[0] = UNUSED;
            state.regfile[1] = UNUSED;
            state.regfile[2] = UNUSED;
            state.regfile[3] = UNUSED;
            // Load the child into R0
            // Check if its already in R0???
            // No optimizations yet.
            int reg = emit_expression_node(node.as.stmt._return.expression);
            // Write it into return value slot, which 
            emitf("\n");
            emitf("STR R%d, R5, #3 ; Write return value, always R5 + 3\n", reg);
            // Need to know what funciton we are returning from somehow.
            emitf("BRnzp %s_teardown \n", current_function_name);
            emitf("\n");
            return;
        }
        case A_IF_STMT: {
            state.regfile[0] = UNUSED;
            state.regfile[1] = UNUSED;
            state.regfile[2] = UNUSED;
            state.regfile[3] = UNUSED;
            int32_t r_condition = emit_expression_node(node.as.stmt._if.condition);
            // Test the condition code
            // TODO: Else statement.
            emitf("AND R%d, R%d, R%d  ; Load condition expr result into NZP\n", r_condition, r_condition, r_condition);
            //emitf(r_condition);
            emitf("BRnz if_stmt%d_end ; Jump over if block if condition is false\n", if_counter);
            emit_ast_node(node.as.stmt._if.if_stmt);
            emitf("if_stmt%d_end\n", if_counter++);
            return;
        }
        case A_COMPOUND_STMT: {
            state.regfile[0] = UNUSED;
            state.regfile[1] = UNUSED;
            state.regfile[2] = UNUSED;
            state.regfile[3] = UNUSED;
            for (int i = 0; i < (node.as.stmt.compound.statements.size); i++)
                emit_ast_node(node.as.stmt.compound.statements.data[i]);
            return;
        }
        case A_FUNCTION_DECL: {
            bool is_main = !strcmp(node.as.func_decl.identifier, "main");
            state.regfile[0] = UNUSED;
            state.regfile[1] = UNUSED;
            state.regfile[2] = UNUSED;
            state.regfile[3] = UNUSED;
            current_function_name = node.as.func_decl.identifier;
            // Callee save registers:

            // 
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
            // Callee Save Registers
            /**
            if (is_main) {
                emitf("ADD R6, R6, #-4 ; \n");
                emitf("STR R0, R6, #3  ; Push R0 \n");
                emitf("STR R1, R6, #2  ; Push R1\n");
                emitf("STR R2, R6, #1  ; Push R2\n");
                emitf("STR R3, R6, #0  ; Push R3\n");
                }
                */
            
            emitf("ADD R5, R6, #-1 ; Set frame pointer for function\n");
            emitf("\n");
            emitf("; Perform work for function: \n");
            emit_ast_node(node.as.func_decl.body);
            emitf("%s_teardown \n", node.as.func_decl.identifier);
            emitf("ADD R6, R5, #1 ; Pop local variables\n");
            emitf("\n");
            // Callee Restore Registers
            /**
            if (is_main) {
                emitf("ADD R6, R6, #4\n");
                emitf("LDR R0, R6, #-3 ; \n");
                emitf("LDR R1, R6, #-2 ; \n");
                emitf("LDR R2, R6, #-1 ; \n");
                emitf("LDR R3, R6, #0 ; \n");
            }
            */
            

            emitf("LDR R5, R6, #0 ; Pop the frame pointer\n");
            emitf("ADD R6, R6, #1\n");
            emitf("\n");
            emitf("LDR R7, R6, #0 ; Pop the return address\n");
            emitf("ADD R6, R6, #1\n");
            
            // Don't return from main.
            if (!is_main)
                emitf("RET\n");
            emitf("; End function %s\n", node.as.func_decl.identifier);
            emitf("\n");
            return;
        }
        case A_PARAM_DECL: 
            // Don't do anything.
            return;
        case A_VAR_DECL:
            // Is global scope
            // We will have a global data section, instead of placing them in the same order
            // as they are declared in the program.
            // Global data section is at address x5000
            // Global variable
            // This decl needs to point to a 
            if (var_decl_scopes[node_h] == 0) {
                emitf("%s .FILL x0000\n", node.as.var_decl.identifier);
                // No initializer.
                return;
            }
            
            // Local Variable, 
            // allocate space for local variable
            emitf("ADD R6, R6, #-1 ; Allocate space for \"%s\"\n", node.as.var_decl.identifier);
            if (node.as.var_decl.initializer != -1) {
                // TOOD: Assignment nodes should not really be separate, there should be an assignemnt expression.
                // But for now, we can kepe initialization separate, as for static ints it works different I suppose
                // Load a reg with the initializer value
                int reg = emit_expression_node(node.as.var_decl.initializer);
                symbol_table_entry_t symbol = symbol_table_search(var_decl_scopes[node_h], node.as.var_decl.identifier);
                
                emitf("STR R%d, R5, #%d ; Initialize \"%s\" \n\n", reg, symbol.offset, node.as.var_decl.identifier);
                state.regfile[reg] = UNUSED;
            }
            return;
            
        case A_FUNCTION_CALL:
        case A_UNARY_EXPR:
        case A_BINARY_EXPR: 
        case A_INTEGER_LITERAL: 
        case A_SYMBOL_REF: 
            emit_expression_node(node_h);
            return;
        
    }
}