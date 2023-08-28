#include <string.h>

#include "analysis.h"
#include "AST.h"
#include "util/util.h"

analysis_t analysis;

static i32 new_scope() {
    static i32 i = 0;
    return ++i;
}

static i32 curr_scope() {
    return analysis.scopes.scope_stack[analysis.scopes.idx];
}

static void enter_scope() {
    // TODO: Error checking.
    i32 new = new_scope(); 
    analysis.symbol_table.parent_scope[new] = curr_scope();
    analysis.scopes.scope_stack[++analysis.scopes.idx] = new;
}

static void exit_scope() {
    analysis.scopes.idx--;
}

void analysis_exit_ast_node(ast_node_h root) {
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

void analyze_ast_node(ast_node_h node_h) {
    if(node_h == -1)
        return;

    struct AST_NODE_STRUCT node = ast_node_data(node_h);

    // Traverse to the children.
    switch(node.type) {      
        case A_COMPOUND_STMT: {
            if (node.as.stmt.compound.scope_flag == NEWSCOPE) {
                uint16_t parent_scope = curr_scope();
                enter_scope();
                analysis.offsets[curr_scope()] = analysis.offsets[parent_scope];
            }
            break;
        }
        case A_VAR_DECL: {
            symbol_table_add(&(analysis.symbol_table), node.as.var_decl.token, curr_scope(),  node.as.var_decl.type_info, VARIABLE_ST_ENTRY, 1, analysis.offsets[curr_scope()]++);
            analysis.scopes.var_decls[node_h] = curr_scope();
            break;
        }
        case A_PARAM_DECL: {
            symbol_table_add(&(analysis.symbol_table), node.as.param_decl.token, curr_scope(), node.as.param_decl.type_info, PARAMETER_ST_ENTRY, 1, analysis.offsets[curr_scope()]++);
            analysis.scopes.var_decls[node_h] = curr_scope();
            break;
        }
        case A_FUNCTION_DECL: {
            symbol_table_add(&(analysis.symbol_table), node.as.func_decl.token, curr_scope(), node.as.func_decl.type_info, FUNCTION_ST_ENTRY, 1, 1);
            enter_scope();
            break;
        }
        // For statements have there own scope for vairables defined in them. They also have a child scope that is the compound statement / body.
        case A_FOR_STMT: {
            uint16_t parent_scope = curr_scope();
            enter_scope();
            analysis.offsets[curr_scope()] = analysis.offsets[parent_scope];
            break;
        }
        case A_SYMBOL_REF: {
            // Search for it to make sure it exists
            symbol_table_search(&(analysis.symbol_table), node.as.expr.symbol.token, curr_scope());
            analysis.scopes.symbol_refs[node_h] = curr_scope();
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
            printf("Error: analysis traversal unimplemented for this node type.");
            break;
    }
}


