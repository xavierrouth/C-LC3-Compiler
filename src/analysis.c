#include "analysis.h"
#include "AST.h"

#include <string.h>

static symbol_table_t _global_table;
static symbol_table_t* global_table;

// TODO: Support pointing pointers, and more than one type.
// Split static and voltaile etc into things that are not type.
char* type_info_to_str(type_info_t type_info) {
    switch (type_info.type) {
        case INT: return "INT";
        case CHAR: return "CHAR";
        case PTR: return "PTR";
        case VOID: return "VOID";
        default: return "Uknonwn";   
    }
}

symbol_table_t* symbol_table_init(symbol_table_t* parent) {
    symbol_table_t* table = malloc(sizeof(*table));
    table->symbols = symbol_vector_init(16);
    table->children = symbol_table_vector_init(2);
    table->parent = parent;
    return table;
}

void symbol_table_free(symbol_table_t* table) {
    for (int i = 0; i < table->children.size; i++) {
        symbol_table_free(table->children.data[i]);
    }
    free(table);
    return;
}
 
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
            //
            analyze_node(root->as.var_decl.initializer, scope);
            break;
        }
        case A_BINOP_EXPR: {
            analyze_node(root->as.binary_op.left, scope);
            analyze_node(root->as.binary_op.right, scope);
            break;
        }
        case A_FUNCTION_CALL: {
            for (int i = 0; i < (root->as.func_call_expr.arguments.size); i++)
                analyze_node(root->as.func_call_expr.arguments.data[i], scope);
            break;
        }
        case A_FUNCTION_DECL: {
            // The function symbol should go in the main table
            symbol_table_t* func_scope = symbol_table_init(scope);
            symbol_table_entry func_symbol = {0};
            //strncpy(func_symbol.identifier, root->as.func_decl.identifier);
            for (int i = 0; i < (root->as.func_decl.parameters.size); i++) {
                analyze_node(root->as.func_decl.parameters.data[i], func_scope);
            }
                
            analyze_node(root->as.func_decl.body, func_scope);
            break;
        }
        case A_ASSIGN_EXPR: {
            analyze_node(root->as.assign_expr.right, scope);
            break;
        }
        case A_RETURN_STMT: {
            analyze_node(root->as.return_stmt.expression, scope);
            break;
        }
        case A_COMPOUND_STMT: {
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
        case A_VAR_EXPR:
            break;
        default:
            printf("Error: Traversal Unimplemented for this Node type");
            break;
    }
}

void analysis(ast_node_t* root) {
    global_table = &(_global_table);
    strncpy(global_table->name, "global", 15);
    global_table->name[15] = '\0';

    analyze_node(root, global_table);


    return;
}

