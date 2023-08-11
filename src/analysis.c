#include "analysis.h"
#include "AST.h"

#include <string.h>

extern int32_t parent_scope[];

static int32_t scope_stack[8];
static int32_t scope_stack_idx;

static int32_t var_offsets[100];
static int32_t param_offsets[100];

int32_t symbol_ref_scopes[100];
int32_t var_decl_scopes[100];

static int32_t new_scope() {
    static int32_t i = 0;
    return ++i;
}

static int32_t curr_scope() {
    return scope_stack[scope_stack_idx];
}

static void enter_scope() {
    // TODO: Error checking.
    int32_t new = new_scope(); 
    parent_scope[new] = curr_scope();
    scope_stack[++scope_stack_idx] = new;
}

static void exit_scope() {
    scope_stack_idx--;
}

void analysis_exit_ast_node(ast_node_t root) {
    struct AST_NODE_STRUCT node = ast_node_data(root);
    switch(node.type) {      
        case A_COMPOUND_STMT: {
            if (node.as.stmt.compound.scope_flag == NEWSCOPE) {
                exit_scope();
            }
        }
        break;
        case A_FUNCTION_DECL: {
            exit_scope();
        }
        break;
    }
}

void analyze_ast_node(ast_node_t node_h) {
    if(node_h == -1)
        return;

    struct AST_NODE_STRUCT node = ast_node_data(node_h);

    // Traverse to the children.
    switch(node.type) {      
        case A_COMPOUND_STMT: {
            if (node.as.stmt.compound.scope_flag == NEWSCOPE) {
                uint16_t parent_scope = curr_scope();
                enter_scope();
                var_offsets[curr_scope()] = var_offsets[parent_scope];
            }
            break;
        }
        case A_VAR_DECL: {
            symbol_table_add(node.as.var_decl.token, curr_scope(),  node.as.var_decl.type_info, VARIABLE_ST_ENTRY, 1, var_offsets[curr_scope()]++);
            var_decl_scopes[node_h] = curr_scope();
            break;
        }
        case A_PARAM_DECL: {
            symbol_table_add(node.as.param_decl.token, curr_scope(), node.as.param_decl.type_info, PARAMETER_ST_ENTRY, 1, param_offsets[curr_scope()]++);
            var_decl_scopes[node_h] = curr_scope();
            break;
        }
        case A_FUNCTION_DECL: {
            symbol_table_add(node.as.func_decl.token, curr_scope(), node.as.func_decl.type_info, FUNCTION_ST_ENTRY, 1, 1);
            enter_scope();
            break;
        }
        // For statements have there own scope for vairables defined in them. They also have a child scope that is the compound statement / body.
        case A_FOR_STMT: {
            uint16_t parent_scope = curr_scope();
            enter_scope();
            var_offsets[curr_scope()] = var_offsets[parent_scope];
            break;
        }
        case A_SYMBOL_REF: {
            // Search for it to make sure it exists
            symbol_table_search(node.as.expr.symbol.token, curr_scope());
            symbol_ref_scopes[node_h] = curr_scope();
            break;
        }
        case A_INLINE_ASM:
        case A_WHILE_STMT:
        case A_ASSIGN_EXPR: 
        case A_PROGRAM: 
        case A_BINARY_EXPR: 
        case A_FUNCTION_CALL:
        case A_RETURN_STMT:
        case A_UNARY_EXPR: 
        case A_INTEGER_LITERAL:
        case A_IF_STMT:
            break;
        default:
            printf("Error: Analysis traversal Unimplemented for this Node type");
            break;
    }
}


