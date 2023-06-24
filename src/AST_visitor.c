#include "AST.h"
#include "AST_visitor.h"

// Use this same buffer for all the thingies
static char print_buffer[64];

static const char* ast_type_to_str(ast_node_enum type) {
    switch(type) {
        case A_ASSIGN_EXPR: return "A_ASSIGN_EXPR";
        case A_BINOP_EXPR: return "A_BINOP_EXPR";
        case A_COMPOUND_STMT: return "A_COMPOUND_STMT";
        case A_EXPR_STMT: return "A_EXPR_STMT";
        case A_PROGRAM: return "A_PROGRAM";
        case A_VAR_DECL: return "A_VAR_DECL";
        case A_INTEGER_LITERAL: return "A_INTEGER_LITERAL";
        case A_VAR_EXPR: return "A_VAR_EXPR";
        case A_FUNCTION_DECL: return "A_FUNCTION_DECL";
        case A_RETURN_STMT: return "A_RETURN_STMT";
        case A_FUNCTION_CALL: return "A_FUNCTION_CALL";
    }
    // Should probably clear the buffer lol.
    memset(print_buffer, 0, 64);
    snprintf(print_buffer, 64, "%d", type);
    return print_buffer;
}

static const char* ast_op_to_str(ast_op_enum type) {
    switch(type) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_NOT: return "!";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
    }
    snprintf(print_buffer, 64, "%d", type);
    return print_buffer;
}

// This is disgusting I regret everything.
static ast_node_visitor* visitor_print_init() {
    ast_node_visitor* visitor = malloc(sizeof(ast_node_visitor));
    visitor->visitor_type = PRINT_AST;
    visitor->traversal_type = PREORDER;
    visitor->as.print_ast.indentation = -1;
    visitor->as.print_ast.func = &print_ast_node;
    return visitor;
}

static ast_node_visitor* visitor_free_init() {
    ast_node_visitor* visitor = malloc(sizeof(ast_node_visitor));
    visitor->visitor_type = FREE_AST;
    visitor->traversal_type = POSTORDER;
    visitor->as.free_ast.func = &free_ast_node;
    return visitor;
}

static ast_node_visitor* visitor_check_init() {
    ast_node_visitor* visitor = malloc(sizeof(ast_node_visitor));
    visitor->visitor_type = CHECK_AST;
    visitor->traversal_type = PREORDER;
    return visitor;
}

// Call the correct function with the correct parameters
// Why are we using function pointers if we are just going to wrap this in a switch statement anyway...
// We can just call the named functions instead....
// Storing the parameter information inside a visitor struct is interesting 
// and maybe actually useful though.
// What is wrong w me.
static void visitor_call(ast_node_visitor* visitor, ast_node_t* node) {
    switch (visitor->visitor_type) {
        case PRINT_AST: {
            visitor->as.print_ast.func(node, visitor->as.print_ast.indentation);
            return;
        }
        case FREE_AST: {
            visitor->as.free_ast.func(node);
            return;
        }
        case CHECK_AST: {
            visitor->as.check_ast.results[visitor->as.check_ast.index++] = node->type;
            return;
        }
        case EMIT_AST: {
            visitor->as.emit_ast.func(node, &(visitor->as.emit_ast.state));
            return;
        }
    }
}

static void visitor_begin(ast_node_visitor* visitor, ast_node_t* node) {
    switch (visitor->visitor_type) {
        case PRINT_AST: {
            visitor->as.print_ast.indentation++;
            return;
        }
        case EMIT_AST: {
            if (node->type == A_PROGRAM) {
                printf(".ORIG x3000\n");
            }
            return;
        }
    }
}
static void visitor_end(ast_node_visitor* visitor, ast_node_t* node) {
    // Decrement indentation?
    switch (visitor->visitor_type) {
        case PRINT_AST: {
            visitor->as.print_ast.indentation--;
            return;
        }
    }
}

void free_ast_node(ast_node_t* node) {
    switch(node->type) {
        case A_PROGRAM: {
            ast_node_list_free(node->as.program.body);
            free(node);
            return;
        }
        case A_FUNCTION_DECL: {
            ast_node_list_free(node->as.func_decl.parameters);
            free(node);
            return;
        }
        case A_COMPOUND_STMT: {
            ast_node_list_free(node->as.commpound_stmt.statements);
            free(node);
            return;
        }
        case A_FUNCTION_CALL: {
            ast_node_list_free(node->as.func_call_expr.arguments);
            free(node);
            return;
        }
        // The rest:
        case A_VAR_DECL:
        case A_ASSIGN_EXPR:
        case A_BINOP_EXPR:
        case A_RETURN_STMT:
        case A_INTEGER_LITERAL:
        case A_VAR_EXPR:
            free(node);
            return;
        default:
            printf("free_ast_node() unimplemented for this node type.\n");
            return;
    }
    return;
}

// Ideally bool would be templated by traversal_type T/F.
// Then we don't have to repeat code but we also get two versions that are both fast.
// 'branch predictor will take care of it'


void ast_traversal(ast_node_t* root, ast_node_visitor* visitor) {
    //visitor_begin(visitor, root);
    if(root == NULL)
        return;

    visitor_begin(visitor, root);
    if (visitor->traversal_type == PREORDER)
        visitor_call(visitor, root); // Do whatever other stuff we want this node;

    // Traverse to the children.
    switch(root->type) {      
        case A_PROGRAM: {
            for (int i = 0; i < (root->as.program.body.size); i++)
                ast_traversal(root->as.program.body.nodes[i], visitor);
            break;
        }
        case A_VAR_DECL: {
            ast_traversal(root->as.var_decl.initializer, visitor);
            break;
        }
        case A_BINOP_EXPR: {
            ast_traversal(root->as.binary_op.left, visitor);
            ast_traversal(root->as.binary_op.right, visitor);
            break;
        }
        case A_FUNCTION_CALL: {
            for (int i = 0; i < (root->as.func_call_expr.arguments.size); i++)
                ast_traversal(root->as.func_call_expr.arguments.nodes[i], visitor);
            break;
        }
        case A_FUNCTION_DECL: {
            for (int i = 0; i < (root->as.func_decl.parameters.size); i++)
                ast_traversal(root->as.func_decl.parameters.nodes[i], visitor);
            ast_traversal(root->as.func_decl.body, visitor);
            break;
        }
        case A_ASSIGN_EXPR: {
            ast_traversal(root->as.assign_expr.right, visitor);
            break;
        }
        case A_RETURN_STMT: {
            ast_traversal(root->as.return_stmt.expression, visitor);
            break;
        }
        case A_COMPOUND_STMT: {
            for (int i = 0; i < (root->as.commpound_stmt.statements.size); i++)
                ast_traversal(root->as.commpound_stmt.statements.nodes[i], visitor);
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

    if (visitor->traversal_type == POSTORDER)
        visitor_call(visitor, root); // Do whatever other stuff we want this node;
    visitor_end(visitor, root);
    return;
}

void print_ast(ast_node_t* root) {
    ast_node_visitor* visitor = visitor_print_init();
    ast_traversal(root, visitor);
    free(visitor);
    return;
}

void free_ast(ast_node_t* root) {
    ast_node_visitor* visitor = visitor_free_init();
    ast_traversal(root, visitor);
    free(visitor);
    return;
}

void check_ast(ast_node_t* root, ast_node_enum* results) {
    ast_node_visitor* visitor = visitor_check_init();
    visitor->as.check_ast.results = results;
    visitor->as.check_ast.index = 0;
    ast_traversal(root, visitor);
    free(visitor);
    return;
}

void print_ast_node(ast_node_t* node, int indentation) {
    switch (node->type) {
        case A_PROGRAM: {
            snprintf(print_buffer, 64, 
                "<node=%s>\n", \
                ast_type_to_str(node->type));
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_VAR_DECL: {
            snprintf(print_buffer, 64,
                "<node=%s, identifier=\"%s\", type=%s>\n", \
                ast_type_to_str(node->type), 
                node->as.var_decl.identifier,
                type_info_to_str(node->as.var_decl.type_info));   
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_VAR_EXPR: {
           snprintf(print_buffer, 64,
                "<node=%s, identifier=\"%s\">\n", \
                ast_type_to_str(node->type), 
                node->as.var_ref_expr.identifier);   
            printf_indent(indentation*3, print_buffer);
            return;    
        }
        case A_INTEGER_LITERAL: {
            snprintf(print_buffer, 64,
                "<node=%s, value=\"%d\">\n", \
                ast_type_to_str(node->type), 
                node->as.literal.value);
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_FUNCTION_DECL: {
            snprintf(print_buffer, 64,
                "<node=%s, identifier=\"%s\">\n", \
                ast_type_to_str(node->type), 
                node->as.func_decl.identifier);
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_BINOP_EXPR: {
            snprintf(print_buffer, 64,
                "<node=%s, op_type=\"%s\">\n", \
                ast_type_to_str(node->type), 
                ast_op_to_str(node->as.binary_op.type));
            printf_indent(indentation*3, print_buffer);
            return;
        }
        case A_ASSIGN_EXPR: {
            snprintf(print_buffer, 64,
                "<node=%s, identifier=\"%s\">\n", \
                ast_type_to_str(node->type), 
                node->as.assign_expr.identifier);
            printf_indent(indentation*3, print_buffer);
            return;
        }
        case A_RETURN_STMT: {
            snprintf(print_buffer, 64,
                "<node=%s>\n", \
                ast_type_to_str(node->type));
            printf_indent(indentation*3, print_buffer);
            return;
        }
        case A_COMPOUND_STMT: {
            snprintf(print_buffer, 64,
                "<node=%s, num_statements=\"%d\">\n", \
                ast_type_to_str(node->type),
                node->as.commpound_stmt.statements.size);
            printf_indent(indentation*3, print_buffer);
            return;
        }
        case A_FUNCTION_CALL: {
            snprintf(print_buffer, 64,
                "<node=%s, identifier=\"%s\">\n", \
                ast_type_to_str(node->type),
                node->as.func_call_expr.identifier);
            printf_indent(indentation*3, print_buffer);
            return;
        }
        default:
           if (sprintf(print_buffer, "<node=%s, contents=\"%s\">\n", \
                ast_type_to_str(node->type), 
                "No contents yet."))
                printf_indent(indentation*3, print_buffer);
            return;  

    }
}