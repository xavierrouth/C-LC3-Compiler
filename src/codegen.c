#include "codegen.h"
#include "AST_visitor.h"
#include "AST.h"

static ast_node_visitor* visitor_emit_init() {
    ast_node_visitor* visitor = malloc(sizeof(ast_node_visitor));
    visitor->visitor_type = EMIT_AST;
    visitor->traversal_type = POSTORDER;
    visitor->as.emit_ast.func = &emit_ast_node;
    codegen_state_t state = {0};
    visitor->as.emit_ast.state = state; 
    return visitor;
}

/**
 * Code snippits for things I guess
*/
void emit_ast(ast_node_t* root) {
    ast_node_visitor* visitor = visitor_emit_init();
    ast_traversal(root, visitor);
    free(visitor);
    return;
} 

void emit_ast_node(ast_node_t* node, codegen_state_t* const state) {
    switch (node->type) {
        case A_PROGRAM: {
            printf("HALT\n");
            printf(".END\n");
            return;
        }
        case A_ASSIGN_EXPR: {
            // Needs to have access to the children.
            // Need to pass a reference of the prev results (child nodes) through the function call,
            // using 
            // This requires a symbol ref. We can't do those yet.
            printf("");
            return;
        }
        // Somehow we need to pattern match these two into the same instruction.
        // Binops on immediates can be one instruction if the immediate is small enough.
        // How does one even do that?
        case A_RETURN_STMT: {
            // Load the child into R0
            // Check if its already in R0???
            // No optimizations yet.
            printf("ADD R0, R%d, x0000\n", state->ops[state->op_idx % 2]); 
            return;
        }
        case A_BINOP_EXPR: {
            // Only handle addition for now.
            int dest = state->regfile_idx;
            printf("ADD R%d, R%d, R%d\n", dest, state->ops[0], state->ops[1]);
            // Clober the op registers.
            state->ops[state->op_idx++ % 2] = state->regfile_idx;
            state->regfile_idx++;

            // How do we deallocate registesr?? Who knows.
            // I think we should just go with a linear search 
            // Clear last two registers
            return;
        }
        case A_INTEGER_LITERAL: {
            // The literal can be loaded into a register, or it can be an immediate.
            state->regfile[state->regfile_idx] = true;
            // 0 Out than load from constant pool or add 
            int val = node->as.literal.value;
            if (val > 15) {
                // TODO:
                printf("Generating big immediates not supported yet.\n");
            }
            printf("AND R%d, R%d, x0000\n", state->regfile_idx, state->regfile_idx);
            printf("ADD R%d, R%d, #%d\n", state->regfile_idx, state->regfile_idx, val);
            state->ops[state->op_idx++ % 2] = state->regfile_idx; 
            state->regfile_idx++;
            return;  
        }
        case A_VAR_EXPR: {
            state->regfile[state->regfile_idx] = true;
            printf("Store R%d <- %s\n",  state->regfile_idx, node->as.var_ref_expr.identifier);
            state->ops[state->op_idx++ % 2] = state->regfile_idx; 
            state->regfile_idx++;
            return;
        }

    }
}