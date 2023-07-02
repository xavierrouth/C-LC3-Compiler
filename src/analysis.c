#include "analysis.h"
#include "AST.h"

#include <string.h>

extern symtable_vector symtable_root;
 
static void analyze_node(ast_node_t root, int scope) {
    //visitor_begin(visitor, root);
    if(root == -1)
        return;

    struct AST_NODE_STRUCT n = ast_node_data(root);

    // Traverse to the children.
    switch(n.type) {      
        case A_PROGRAM: {
            for (int i = 0; i < (n.as.program.body.size); i++)
                analyze_node(n.as.program.body.data[i], scope);
            break;
        }
        case A_VAR_DECL: {
            // Set the scope in the node to the correct symbol table.
            n.as.var_decl.scope = scope;

            // Create the symbol table entry:
            symtable_entry var_decl = {0};
            // TODO: Search only current scope to make sure this symbol doesn't already exist.
            var_decl.identifier = n.as.var_decl.identifier;
            var_decl.type_info = n.as.var_decl.type_info;
            var_decl.size = 1;
            
            symbol_vector_push(&(symtable_root.data[scope].symbols), var_decl);
            analyze_node(n.as.var_decl.initializer, scope);
            break;
        }
        case A_BINARY_EXPR: {
            analyze_node(n.as.expr.binary.left, scope);
            analyze_node(n.as.expr.binary.right, scope);
            break;
        }
        case A_FUNCTION_CALL: {
            for (int i = 0; i < (n.as.expr.call.arguments.size); i++)
                analyze_node(n.as.expr.call.arguments.data[i], scope);
            break;
        }
        case A_FUNCTION_DECL: {
            // Tell the node the scope of this declaration.
            n.as.func_decl.scope = scope;

            // Make a symbol to go in our current symbol table
            symtable_entry func_symbol = {0};
            func_symbol.identifier = n.as.func_decl.identifier;
            func_symbol.size = 1; // This should never be used for a function.
            func_symbol.type = FUNCTION;
            func_symbol.type_info = n.as.func_decl.type_info;

            symbol_vector_push(&(symtable_root.data[scope].symbols), func_symbol);

            // Make a scope for the function's parameters:
            int func_scope = symtable_branch_init(scope, n.as.func_decl.identifier);

            // Add the scope
            //symtable_vector_push(&(scope->children), func_scope);
            for (int i = 0; i < (n.as.func_decl.parameters.size); i++) {
                analyze_node(n.as.func_decl.parameters.data[i], func_scope);
            }
            
            // Analyze the compound stmt.
            // This will create another new scope, that is a child of the parameters scope.
            analyze_node(n.as.func_decl.body, func_scope);
            break;
        }
        case A_ASSIGN_EXPR: {
            analyze_node(n.as.expr.assign.right, scope);
            analyze_node(n.as.expr.assign.left, scope);
            break;
        }
        case A_RETURN_STMT: {
            analyze_node(n.as.stmt._return.expression, scope);
            break;
        }
        case A_COMPOUND_STMT: {
            int stmt_scope = scope;
            if (n.as.stmt.compound.scope_flag == NEWSCOPE) {
                stmt_scope = symtable_branch_init(scope, "compound_stmt");
            }
            for (int i = 0; i < (n.as.stmt.compound.statements.size); i++)
                analyze_node(n.as.stmt.compound.statements.data[i], stmt_scope);
            break;
        }
        case A_UNARY_EXPR: {
            analyze_node(n.as.expr.unary.child, scope);
            break;
        }
        // Terminal nodes:
        case A_INTEGER_LITERAL:
            break;
        case A_SYMBOL_REF:
            // TODO: Search the current scope to make sure this variable exists.
            n.as.expr.symbol.scope = scope;
            break;
        default:
            printf("Error: Traversal Unimplemented for this Node type");
            break;
    }
}

void analysis(ast_node_t root) {
    analyze_node(root, 0);
    return;
}

