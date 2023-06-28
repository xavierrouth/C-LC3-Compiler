#include "AST.h"
#include "token.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

extern char id_buffer[];

ast_op_enum token_to_op(token_enum type) {
    switch (type) {
        case T_SUB: return OP_SUB;
        case T_ADD: return OP_ADD;
        case T_MUL: return OP_MUL;
        case T_COMMA: return OP_COMMA;
        //case T_NOT: return OP_NOT;
        case T_DIV: return OP_DIV;
        default: return OP_INVALID;
    }
    
}

// Sometimes we init and then free, so we will jump over some labels.
// Oh well!
ast_node_t* ast_node_init() {
    static int node_labels = 0;
    ast_node_t* node = malloc(sizeof(*node));
    memset(node, 0, sizeof(ast_node_t));
    node->type = A_UNKNOWN;
    node->size = 1;
    node->index = node_labels++; 
    return node;
}

// TOOD: AST Node initializers
ast_node_t* ast_return_stmt_init(ast_node_t* expression) {
    ast_node_t* node = ast_node_init();
    node->type = A_RETURN_STMT;
    node->as.return_stmt.expression = expression;
    return node;
}

ast_node_t* ast_program_init(ast_node_vector body) {
    ast_node_t* node = ast_node_init();
    node->type = A_PROGRAM;
    node->as.program.body = body;
    return node;
}

ast_node_t* ast_func_decl_init(ast_node_t* body, ast_node_vector parameters, type_info_t type_info, char* identifier) {
    ast_node_t* node = ast_node_init();
    node->as.func_decl.parameters = parameters;
    node->as.func_decl.body = body;
    
    return body;
}

// Use this same buffer for all the thingies
static char print_buffer[128];

static const char* ast_type_to_str(ast_node_enum type) {
    switch(type) {
        case A_ASSIGN_EXPR: return "A_ASSIGN_EXPR";
        case A_BINOP_EXPR: return "A_BINOP_EXPR";
        case A_COMPOUND_STMT: return "A_COMPOUND_STMT";
        case A_EXPR_STMT: return "A_EXPR_STMT";
        case A_PROGRAM: return "A_PROGRAM";
        case A_VAR_DECL: return "A_VAR_DECL";
        case A_INTEGER_LITERAL: return "A_INTEGER_LITERAL";
        case A_SYMBOL_REF: return "A_SYMBOL_REF";
        case A_FUNCTION_DECL: return "A_FUNCTION_DECL";
        case A_RETURN_STMT: return "A_RETURN_STMT";
        case A_FUNCTION_CALL: return "A_FUNCTION_CALL";
        case A_UNOP_EXPR: return "A_UNOP_EXPR";
    }
    // Should probably clear the buffer lol.
    printf("This is stupid if you get here you need to implement \
     ast_type_to_str for this node\n");
    memset(print_buffer, 0, 128);
    snprintf(print_buffer, 128, "%d", type);
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
    snprintf(print_buffer, 128, "%d", type);
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
    }
}

static void visitor_begin(ast_node_visitor* visitor, ast_node_t* node) {
    switch (visitor->visitor_type) {
        case PRINT_AST: {
            visitor->as.print_ast.indentation++;
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
                ast_traversal(root->as.program.body.data[i], visitor);
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
            ast_traversal(root->as.func_call.symbol_ref, visitor);
            for (int i = 0; i < (root->as.func_call.arguments.size); i++)
                ast_traversal(root->as.func_call.arguments.data[i], visitor);
            break;
        }
        case A_FUNCTION_DECL: {
            for (int i = 0; i < (root->as.func_decl.parameters.size); i++)
                ast_traversal(root->as.func_decl.parameters.data[i], visitor);
            ast_traversal(root->as.func_decl.body, visitor);
            break;
        }
        case A_ASSIGN_EXPR: {
            ast_traversal(root->as.assign_expr.right, visitor);
            ast_traversal(root->as.assign_expr.left, visitor);
            break;
        }
        case A_RETURN_STMT: {
            ast_traversal(root->as.return_stmt.expression, visitor);
            break;
        }
        case A_COMPOUND_STMT: {
            for (int i = 0; i < (root->as.commpound_stmt.statements.size); i++)
                ast_traversal(root->as.commpound_stmt.statements.data[i], visitor);
            break;
        }
        case A_UNOP_EXPR: {
            ast_traversal(root->as.unary_op.child, visitor);
            break;
        }
        // Terminal nodes:
        case A_INTEGER_LITERAL:
        case A_SYMBOL_REF:
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
            snprintf(print_buffer, 128, 
                "<node=%s, size=%d, index=%d>\n", \
                ast_type_to_str(node->type),
                node->size,
                node->index);
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_VAR_DECL: {
            snprintf(print_buffer, 128,
                "<node=%s, identifier=\"%s\", type=%s, size=%d, index=%d>\n", \
                ast_type_to_str(node->type), 
                node->as.var_decl.identifier,
                type_info_to_str(node->as.var_decl.type_info),
                node->size,
                node->index);   
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_SYMBOL_REF: {
           snprintf(print_buffer, 128,
                "<node=%s, identifier=\"%s\">, size=%d, index=%d>\n", \
                ast_type_to_str(node->type), 
                node->as.symbol_ref.identifier,
                node->size,
                node->index);   
            printf_indent(indentation*3, print_buffer);
            return;    
        }
        case A_INTEGER_LITERAL: {
            snprintf(print_buffer, 128,
                "<node=%s, value=\"%d\", size=%d, index=%d>\n", \
                ast_type_to_str(node->type), 
                node->as.literal.value,
                node->size,
                node->index);
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_FUNCTION_DECL: {
            snprintf(print_buffer, 128,
                "<node=%s, identifier=\"%s\", size=%d, index=%d>\n", \
                ast_type_to_str(node->type), 
                node->as.func_decl.identifier,
                node->size,
                node->index);
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_BINOP_EXPR: {
            snprintf(print_buffer, 128,
                "<node=%s, op_type=\"%s\", size=%d, index=%d>\n", \
                ast_type_to_str(node->type), 
                ast_op_to_str(node->as.binary_op.type),
                node->size,
                node->index);
            printf_indent(indentation*3, print_buffer);
            return;
        }
        case A_UNOP_EXPR: {
            snprintf(print_buffer, 128,
                "<node=%s, op_type=\"%s\", size=%d, index=%d>\n", \
                ast_type_to_str(node->type), 
                ast_op_to_str(node->as.unary_op.type),
                node->size,
                node->index);
            printf_indent(indentation*3, print_buffer);
            return;
        }
        case A_ASSIGN_EXPR: {
            snprintf(print_buffer, 128,
                "<node=%s, size=%d, index=%d>\n", \
                ast_type_to_str(node->type), 
                node->size,
                node->index);
            printf_indent(indentation*3, print_buffer);
            return;
        }
        case A_RETURN_STMT: {
            snprintf(print_buffer, 128,
                "<node=%s, size=%d, index=%d>\n", \
                ast_type_to_str(node->type),
                node->size,
                node->index);
            printf_indent(indentation*3, print_buffer);
            return;
        }
        case A_COMPOUND_STMT: {
            snprintf(print_buffer, 128,
                "<node=%s, num_statements=\"%d\", size=%d, index=%d>\n", \
                ast_type_to_str(node->type),
                node->as.commpound_stmt.statements.size,
                node->size,
                node->index);
            printf_indent(indentation*3, print_buffer);
            return;
        }
        case A_FUNCTION_CALL: {
            snprintf(print_buffer, 128,
                "<node=%s, , size=%d, index=%d>\n", \
                ast_type_to_str(node->type),
                node->size,
                node->index);
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

void free_ast_node(ast_node_t* node) {
    switch(node->type) {
        case A_PROGRAM: {
            ast_node_vector_free(node->as.program.body);
            free(node);
            return;
        }
        case A_FUNCTION_DECL: {
            ast_node_vector_free(node->as.func_decl.parameters);
            free(node);
            return;
        }
        case A_COMPOUND_STMT: {
            ast_node_vector_free(node->as.commpound_stmt.statements);
            free(node);
            return;
        }
        case A_FUNCTION_CALL: {
            ast_node_vector_free(node->as.func_call.arguments);
            free(node);
            return;
        }
        // The rest:
        case A_VAR_DECL:
        case A_ASSIGN_EXPR:
        case A_BINOP_EXPR:
        case A_RETURN_STMT:
        case A_INTEGER_LITERAL:
        case A_SYMBOL_REF:
        case A_UNOP_EXPR:
            free(node);
            return;
        default:
            printf("free_ast_node() unimplemented for this node type.\n");
            return;
    }
    return;
}