#include "AST.h"
#include "token.h"
#include "util.h"
#include "symbol_table.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define errorf printf

#define MAX_NUM_AST_NODES 100

extern char id_buffer[];

// Give other files access to this.
static struct AST_NODE_STRUCT ast_instances[MAX_NUM_AST_NODES];
bool ast_instance_live[MAX_NUM_AST_NODES];

struct AST_NODE_STRUCT ast_node_data(ast_node_t node) {
    // Returns the underlying data
    if (!(node < MAX_NUM_AST_NODES)) {
        errorf("Accessing invalid ast node\n");
        return ast_instances[0];
    }
    return ast_instances[node];
}

ast_node_t ast_node_init(ast_node_enum type) {
    // TODO: Optimize this:
    static uint32_t ast_instance_last = 0;

    for (uint32_t j = 0, i = ast_instance_last; i < MAX_NUM_AST_NODES; j++, i = j % MAX_NUM_AST_NODES) {
        if (!ast_instance_live[i]) {
            ast_instance_live[i] = true;
            ast_instances[i].type = type;
            ast_instance_last = i;
            return i;
        }
    }
    errorf("Ran out of instances\n");
    return -1; // Error here.
}

// TOOD: AST Node initializers
ast_node_t ast_int_literal_init(uint32_t value) {
    ast_node_t node = ast_node_init(A_INTEGER_LITERAL);
    ast_instances[node].as.expr.literal.value = value;
    return node;
}

// Initialize a function call known arguments.
ast_node_t ast_expr_call_init(ast_node_t symbol_ref, ast_node_vector arguments) {
    ast_node_t node = ast_node_init(A_FUNCTION_CALL);
    //if (node == -1)
        // TODO: Error handling.
    ast_instances[node].as.expr.call.symbol_ref = symbol_ref;
    ast_instances[node].as.expr.call.arguments = arguments;
    return node;
}

ast_node_t ast_expr_symbol_init(char* identifier, int scope) {
    ast_node_t node = ast_node_init(A_SYMBOL_REF);
    ast_instances[node].as.expr.symbol.identifier = identifier;
    ast_instances[node].as.expr.symbol.scope = scope;
    // Why does a symbol reference have a type??
    //ast_instances[node].as.expr.symbol.type = type;
    return node;
}

ast_node_t ast_assign_expr_init(ast_node_t left, ast_node_t right) {
    ast_node_t node = ast_node_init(A_ASSIGN_EXPR);
    ast_instances[node].as.expr.assign.left = left;
    ast_instances[node].as.expr.assign.right = right;
    return node;
}

ast_node_t ast_unary_op_init(ast_op_enum type, ast_node_t child, bool order) {
    ast_node_t node = ast_node_init(A_UNARY_EXPR);
    ast_instances[node].as.expr.unary.type = type;
    ast_instances[node].as.expr.unary.child = child;
    ast_instances[node].as.expr.unary.order = order;
    return node;
}

ast_node_t ast_binary_op_init(ast_op_enum type, ast_node_t left, ast_node_t right) {
    ast_node_t node = ast_node_init(A_BINARY_EXPR);
    ast_instances[node].as.expr.binary.type = type;
    ast_instances[node].as.expr.binary.left = left;
    ast_instances[node].as.expr.binary.right = right;
    return node;
}

ast_node_t ast_ternary_op_init(ast_op_enum type, ast_node_t left, ast_node_t right) {
    ast_node_t node = ast_node_init(A_TERNARY_EXPR);
    ast_instances[node].as.expr.binary.type = type;
    ast_instances[node].as.expr.binary.left = left;
    ast_instances[node].as.expr.binary.right = right;
    return node;
}

ast_node_t ast_compound_stmt_init(ast_node_vector statements, bool scope_flag) {
    ast_node_t node = ast_node_init(A_COMPOUND_STMT);
    ast_instances[node].as.stmt.compound.statements = statements;
    ast_instances[node].as.stmt.compound.scope_flag = scope_flag;
    return node;
}

ast_node_t ast_return_stmt_init(ast_node_t expression) {
    ast_node_t node = ast_node_init(A_RETURN_STMT);
    ast_instances[node].as.stmt._return.expression = expression;
    return node;
}

// TODO: For loop, if statement, decl_statement

// -1 for no initializer
ast_node_t ast_var_decl_init(ast_node_t initializer, type_info_t type_info, char* identifier) {
    ast_node_t node = ast_node_init(A_VAR_DECL);
    ast_instances[node].as.var_decl.identifier = identifier;
    ast_instances[node].as.var_decl.initializer = initializer;
    ast_instances[node].as.var_decl.type_info = type_info;
    return node;
}

ast_node_t ast_param_decl_init(type_info_t type_info, char* identifier) {
    ast_node_t node = ast_node_init(A_PARAM_DECL);
    ast_instances[node].as.param_decl.identifier = identifier;
    ast_instances[node].as.param_decl.type_info = type_info;
    return node;
}

ast_node_t ast_func_decl_init(ast_node_t body, ast_node_vector parameters, type_info_t type_info, char* identifier) {
    ast_node_t node = ast_node_init(A_FUNCTION_DECL);
    ast_instances[node].as.func_decl.identifier = identifier;
    ast_instances[node].as.func_decl.body = body;
    ast_instances[node].as.func_decl.type_info = type_info;
    ast_instances[node].as.func_decl.parameters = parameters;  
    return node;
}

ast_node_t ast_program_init(ast_node_vector body) {
    ast_node_t node = ast_node_init(A_PROGRAM);
    ast_instances[node].as.program.body = body;
    return node;
}

// Use this same buffer for all the thingies
extern char print_buffer[128];

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

static const char* ast_type_to_str(ast_node_enum type) {
    switch(type) {
        case A_ASSIGN_EXPR: return "A_ASSIGN_EXPR";
        case A_BINARY_EXPR: return "A_BINARY_EXPR";
        case A_COMPOUND_STMT: return "A_COMPOUND_STMT";
        case A_PROGRAM: return "A_PROGRAM";
        case A_VAR_DECL: return "A_VAR_DECL";
        case A_PARAM_DECL: return "A_PARAM_DECL";
        case A_INTEGER_LITERAL: return "A_INTEGER_LITERAL";
        case A_SYMBOL_REF: return "A_SYMBOL_REF";
        case A_FUNCTION_DECL: return "A_FUNCTION_DECL";
        case A_RETURN_STMT: return "A_RETURN_STMT";
        case A_FUNCTION_CALL: return "A_FUNCTION_CALL";
        case A_UNARY_EXPR: return "A_UNOP_EXPR";
        case A_TERNARY_EXPR: return "A_TERNARY_EXPR";
    }
    return "ast to string unimlpemented";
}

// These need to be strings because we will have ops that are more than one char wide.
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

// Visitor pattern:
static ast_node_visitor visitor_free_init() {
    ast_node_visitor visitor;
    visitor.visitor_type = FREE_AST;
    visitor.traversal_type = POSTORDER;
    return visitor;
}

// Only one visitor at a time
static ast_node_visitor visitor_print_init() {
    ast_node_visitor visitor;
    visitor.visitor_type = PRINT_AST;
    visitor.traversal_type = PREORDER;
    visitor.as.print_ast.indentation = -1;
    return visitor;
}

static ast_node_visitor visitor_check_init() {
    ast_node_visitor visitor;
    visitor.visitor_type = CHECK_AST;
    visitor.traversal_type = PREORDER;
    return visitor;
}


// print_ast_node 

static void visitor_call(ast_node_t node, ast_node_visitor* visitor) {
    switch (visitor->visitor_type) {
        case PRINT_AST: {
            print_ast_node(node, visitor->as.print_ast.indentation);
            return;
        }
        case FREE_AST: {
            free_ast_node(node);
            return;
        }
        case CHECK_AST: {
            visitor->as.check_ast.results[visitor->as.check_ast.index++] = ast_instances[node].type;
            return;
        }
    }
}

static void visitor_begin(ast_node_t node, ast_node_visitor* visitor) {
    switch (visitor->visitor_type) {
        case PRINT_AST: {
            visitor->as.print_ast.indentation++;
            return;
        }
    }
}

static void visitor_end(ast_node_t node, ast_node_visitor* visitor) {
    // Decrement indentation?
    switch (visitor->visitor_type) {
        case PRINT_AST: {
            visitor->as.print_ast.indentation--;
            return;
        }
    }
}

void ast_traversal(ast_node_t root, ast_node_visitor* visitor) {
    if(root == -1)
        return;

    visitor_begin(root, visitor);
    if (visitor->traversal_type == PREORDER)
        visitor_call(root, visitor); // Do whatever other stuff we want this node;

    // Traverse to the children.
    switch(ast_instances[root].type) {      
        case A_PROGRAM: {
            for (int i = 0; i < (ast_instances[root].as.program.body.size); i++)
                ast_traversal(ast_instances[root].as.program.body.data[i], visitor);
            break;
        }
        case A_VAR_DECL: {
            ast_traversal(ast_instances[root].as.var_decl.initializer, visitor);
            break;
        }
        case A_BINARY_EXPR: {
            ast_traversal(ast_instances[root].as.expr.binary.left, visitor);
            ast_traversal(ast_instances[root].as.expr.binary.right, visitor);
            break;
        }
        case A_UNARY_EXPR: {
            ast_traversal(ast_instances[root].as.expr.unary.child, visitor);
            break;
        }
        case A_FUNCTION_CALL: {
            ast_traversal(ast_instances[root].as.expr.call.symbol_ref, visitor);
            for (int i = 0; i < (ast_instances[root].as.expr.call.arguments.size); i++)
                ast_traversal(ast_instances[root].as.expr.call.arguments.data[i], visitor);
            break;
        }
        case A_FUNCTION_DECL: {
            for (int i = 0; i < (ast_instances[root].as.func_decl.parameters.size); i++)
                ast_traversal(ast_instances[root].as.func_decl.parameters.data[i], visitor);
            ast_traversal(ast_instances[root].as.func_decl.body, visitor);
            break;
        }
        case A_ASSIGN_EXPR: {
            ast_traversal(ast_instances[root].as.expr.assign.right, visitor);
            ast_traversal(ast_instances[root].as.expr.assign.left, visitor);
            break;
        }
        case A_RETURN_STMT: {
            ast_traversal(ast_instances[root].as.stmt._return.expression, visitor);
            break;
        }
        case A_COMPOUND_STMT: {
            for (int i = 0; i < (ast_instances[root].as.stmt.compound.statements.size); i++)
                ast_traversal(ast_instances[root].as.stmt.compound.statements.data[i], visitor);
            break;
        }
        // Terminal nodes:
        case A_PARAM_DECL:
        case A_INTEGER_LITERAL:
        case A_SYMBOL_REF:
            break;
        default:
            printf("Error: Traversal Unimplemented for this Node type");
            break;
    }

    if (visitor->traversal_type == POSTORDER)
        visitor_call(root, visitor); // Do whatever other stuff we want this node;
    visitor_end(root, visitor);
    return;
}

void free_ast(ast_node_t root) {
    ast_node_visitor visitor = visitor_free_init();
    ast_traversal(root, &visitor);
    return;
}

void print_ast(ast_node_t root) {
    ast_node_visitor visitor = visitor_print_init();
    ast_traversal(root, &visitor);
    return;
}

void check_ast(ast_node_t root, ast_node_enum* results) {
    ast_node_visitor visitor = visitor_check_init();
    visitor.as.check_ast.results = results;
    visitor.as.check_ast.index = 0;
    ast_traversal(root, &visitor);
    return;
}

void print_ast_node(ast_node_t node, int indentation) {
    switch (ast_instances[node].type) {
        case A_PROGRAM: {
            snprintf(print_buffer, 128, 
                "<node=%s, size=%d>\n", \
                ast_type_to_str(ast_instances[node].type),
                ast_instances[node].size);
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_VAR_DECL: {
            snprintf(print_buffer, 128,
                "<node=%s, identifier=\"%s\", type=%s, size=%d>\n", \
                ast_type_to_str(ast_instances[node].type), 
                ast_instances[node].as.var_decl.identifier,
                type_info_to_str(ast_instances[node].as.var_decl.type_info),
                ast_instances[node].size);   
            printf_indent(indentation*3, print_buffer);
            return;            
        }
         case A_PARAM_DECL: {
            snprintf(print_buffer, 128,
                "<node=%s, identifier=\"%s\", type=%s, size=%d>\n", \
                ast_type_to_str(ast_instances[node].type), 
                ast_instances[node].as.param_decl.identifier,
                type_info_to_str(ast_instances[node].as.param_decl.type_info),
                ast_instances[node].size);   
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_SYMBOL_REF: {
           snprintf(print_buffer, 128,
                "<node=%s, identifier=\"%s\", size=%d>\n", \
                ast_type_to_str(ast_instances[node].type), 
                ast_instances[node].as.expr.symbol.identifier,
                ast_instances[node].size);   
            printf_indent(indentation*3, print_buffer);
            return;    
        }
        case A_INTEGER_LITERAL: {
            snprintf(print_buffer, 128,
                "<node=%s, value=\"%d\", size=%d>\n", \
                ast_type_to_str(ast_instances[node].type), 
                ast_instances[node].as.expr.literal.value,
                ast_instances[node].size);
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_FUNCTION_DECL: {
            snprintf(print_buffer, 128,
                "<node=%s, identifier=\"%s\", size=%d>\n", \
                ast_type_to_str(ast_instances[node].type), 
                ast_instances[node].as.func_decl.identifier,
                ast_instances[node].size);
            printf_indent(indentation*3, print_buffer);
            return;            
        }
        case A_BINARY_EXPR: {
            snprintf(print_buffer, 128,
                "<node=%s, op_type=\"%s\", size=%d>\n", \
                ast_type_to_str(ast_instances[node].type), 
                ast_op_to_str(ast_instances[node].as.expr.binary.type),
                ast_instances[node].size);
            printf_indent(indentation*3, print_buffer);
            return;
        }
        case A_UNARY_EXPR: {
            snprintf(print_buffer, 128,
                "<node=%s, op_type=\"%s\", size=%d>\n", \
                ast_type_to_str(ast_instances[node].type), 
                ast_op_to_str(ast_instances[node].as.expr.unary.type),
                ast_instances[node].size);
            printf_indent(indentation*3, print_buffer);
            return;
        }
        case A_ASSIGN_EXPR: {
            snprintf(print_buffer, 128,
                "<node=%s, size=%d>\n", \
                ast_type_to_str(ast_instances[node].type), 
                ast_instances[node].size);
            printf_indent(indentation*3, print_buffer);
            return;
        }
        case A_RETURN_STMT: {
            snprintf(print_buffer, 128,
                "<node=%s, size=%d>\n", \
                ast_type_to_str(ast_instances[node].type),
                ast_instances[node].size);
            printf_indent(indentation*3, print_buffer);
            return;
        }
        case A_COMPOUND_STMT: {
            snprintf(print_buffer, 128,
                "<node=%s, num_statements=\"%d\", size=%d>\n", \
                ast_type_to_str(ast_instances[node].type),
                ast_instances[node].as.stmt.compound.statements.size,
                ast_instances[node].size);
            printf_indent(indentation*3, print_buffer);
            return;
        }
        case A_FUNCTION_CALL: {
            snprintf(print_buffer, 128,
                "<node=%s, , size=%d>\n", \
                ast_type_to_str(ast_instances[node].type),
                ast_instances[node].size);
            printf_indent(indentation*3, print_buffer);
            return;
        }
        default:
           if (sprintf(print_buffer, "<node=%s, contents=\"%s\">\n", \
                ast_type_to_str(ast_instances[node].type), 
                "No contents yet."))
                printf_indent(indentation*3, print_buffer);
            return;  

    }
}

void free_ast_node(ast_node_t node) {
    switch(ast_instances[node].type) {
        case A_PROGRAM: {
            ast_node_vector_free(ast_instances[node].as.program.body);
            return;
        }
        case A_FUNCTION_DECL: {
            ast_node_vector_free(ast_instances[node].as.func_decl.parameters);
            return;
        }
        case A_COMPOUND_STMT: {
            ast_node_vector_free(ast_instances[node].as.stmt.compound.statements);
            return;
        }
        case A_FUNCTION_CALL: {
            ast_node_vector_free(ast_instances[node].as.expr.call.arguments);
            return;
        }
    }
    return;
}
