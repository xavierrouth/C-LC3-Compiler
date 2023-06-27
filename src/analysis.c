#include "analysis.h"
#include "AST.h"

#include <string.h>

static symbol_table_t _global_table;
static symbol_table_t* global_table;
 
static void analyze_node(ast_node_t* root, symbol_table_t* scope) {
    //visitor_begin(visitor, root);
    if(root == NULL)
        return;

    // Traverse to the children.
    switch(root->type) {      
        case A_PROGRAM: {
            for (int i = 0; i < (root->as.program.body.size); i++)
                analyze_node(root->as.program.body.data[i], scope);
            break;
        }
        case A_VAR_DECL: {
            // Create the symbol table entry:
            symbol_table_entry var_decl = {0};
            // TODO: Search only current scope to make sure this symbol doesn't already exist.
            var_decl.identifier = root->as.var_decl.identifier;
            var_decl.stack_offset = scope->offset--;
            var_decl.type_info = root->as.var_decl.type_info;
            var_decl.parameter = root->as.var_decl.is_parameter;
            
            // Set the scope in the node to the correct symbol table.
            root->as.var_decl.scope = scope;
            symbol_vector_push(&(scope->symbols), var_decl);
            analyze_node(root->as.var_decl.initializer, scope);
            break;
        }
        case A_BINOP_EXPR: {
            analyze_node(root->as.binary_op.left, scope);
            analyze_node(root->as.binary_op.right, scope);
            break;
        }
        case A_FUNCTION_CALL: {
            for (int i = 0; i < (root->as.func_call.arguments.size); i++)
                analyze_node(root->as.func_call.arguments.data[i], scope);
            break;
        }
        case A_FUNCTION_DECL: {
            // Make a symbol to go in our current symbol table
            symbol_table_entry func_symbol = {0};
            func_symbol.identifier = root->as.func_decl.identifier;
            func_symbol.stack_offset = scope->offset--; // Subtract the size of the data type / struct
            func_symbol.type_info = root->as.func_decl.type_info;
            symbol_vector_push(&(scope->symbols), func_symbol);

            // Make a scope for the function's parameters:
            symbol_table_t* func_scope = symbol_table_init(scope);
            func_scope->name = root->as.func_decl.identifier;

            // Tell the node the scope of this declaration.
            root->as.func_decl.scope = scope;

            // Add the scope
            symbol_table_vector_push(&(scope->children), func_scope);
            for (int i = 0; i < (root->as.func_decl.parameters.size); i++) {
                analyze_node(root->as.func_decl.parameters.data[i], func_scope);
            }
            
            // Analyze the compound stmt.
            // This will create another new scope, that is a child of the parameters scope.
            analyze_node(root->as.func_decl.body, func_scope);
            break;
        }
        case A_ASSIGN_EXPR: {
            analyze_node(root->as.assign_expr.right, scope);
            analyze_node(root->as.assign_expr.left, scope);
            break;
        }
        case A_RETURN_STMT: {
            analyze_node(root->as.return_stmt.expression, scope);
            break;
        }
        case A_COMPOUND_STMT: {
            // Create another scope
            // TODO: C scoping means this really should make anew scope, but not implemented.
            // This is dumb, compound stmt shouldn't create a new scope for now.
            for (int i = 0; i < (root->as.commpound_stmt.statements.size); i++)
                analyze_node(root->as.commpound_stmt.statements.data[i], scope);
            break;
        }
        case A_UNOP_EXPR: {
            analyze_node(root->as.unary_op.child, scope);
            break;
        }
        // Terminal nodes:
        case A_INTEGER_LITERAL:
            break;
        case A_SYMBOL_REF:
            // TODO: Search the current scope to make sure this variable exists.
            root->as.symbol_ref.scope = scope;
            break;
        default:
            printf("Error: Traversal Unimplemented for this Node type");
            break;
    }
}

void analysis(ast_node_t* root) {
    global_table = symbol_table_init(NULL);
    global_table->name = "global table";
    
    analyze_node(root, global_table);

    return;
}

