#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "codegen.h"
#include "asmprinter.h"
#include "frontend/analysis.h"
#include "frontend/AST.h"
#include "frontend/symbol_table.h"

// The whole visitor pattern is stupid.

// Register codegen_states
#define UNUSED 0
#define USED 1

static codegen_state_t codegen_state;

// Just know for now that regfile is 8 registesr???/
static uint16_t get_empty_reg() {
    for (uint16_t i = 0; i < 4; i++) {
        if (codegen_state.regfile[i] == UNUSED) {
            return i;
        }
    }
    printf("No empty registers, please write a shorter expression.\n");
    return -1; // Will j break I guess.
}

void emit_ast(ast_node_h root) {
    emit_ast_node(root);
    return;
}

// TODO: Make this better:


static int16_t emit_expression_node(ast_node_h node_h);

// static void get_address(symbol);

// TODO: Optimization
// If the result is unused We can greatly simplify this by just modifying the nzp in the if codegen_statement.
static int16_t emit_condition_node(ast_node_h node_h) {
    struct AST_NODE_STRUCT node = ast_node_data(node_h);

    int16_t right = emit_expression_node(node.as.expr.binary.right);
    codegen_state.regfile[right] = USED;
    int16_t left = emit_expression_node(node.as.expr.binary.left);

    codegen_state.regfile[right] = UNUSED;
    codegen_state.regfile[left] = UNUSED;

    int16_t ret = left;
    int16_t scratch = right;

    // Store result in left always.
    switch (node.as.expr.binary.type) {
        case OP_LT: { // Sub left from right.

            emit_inst_comment((lc3_instruction_t) {.opcode = NOT, .arg1 = left, .arg2 = left}, "evaluate '<'");
            emit_inst((lc3_instruction_t) {.opcode = ADDimm, .arg1 = left, .arg2 = left, .arg3 = 1});
            emit_inst((lc3_instruction_t) {.opcode = ADDreg, .arg1 = ret, .arg2 = left, .arg3 = right});

            break;
        }
        case OP_GT: {

            emit_inst_comment((lc3_instruction_t) {.opcode = NOT, .arg1 = right, .arg2 = right}, "evaluate '>'");
            emit_inst((lc3_instruction_t) {.opcode = ADDimm, .arg1 = right, .arg2 = right, .arg3 = 1});
            emit_inst((lc3_instruction_t) {.opcode = ADDreg, .arg1 = ret, .arg2 = left, .arg3 = right});

            break;
        }
        case OP_EQUALS: {
            //TODO: Optimize this:
            emit_comment("please don't do equalsequals yet I am so sorry");
            #if 0
            emit_inst_comment((lc3_instruction_t) {.opcode = NOT, .arg1 = left, .arg2 = left}, "evaluate '=='");
            emit_inst((lc3_instruction_t) {.opcode = ADDimm, .arg1 = left, .arg2 = left, .arg3 = 1});
            emit_inst((lc3_instruction_t) {.opcode = ADDreg, .arg1 = ret, .arg2 = left, .arg3 = right});

            
            format("NOT R%d, R%d ; Calculate '==' \n", left, left);
            format("ADD R%d, R%d, #1\n", left, left);
            format("ADD R%d, R%d, R%d\n", ret, left, right);
            format("BRz #2\n"); // If its zero, skip the next stepa
            format("AND R%d, R%d, #0\n", ret, ret); // Zero out reg 
            format("BRz #2\n");
            format("AND R%d, R%d, #0\n", ret, ret); // Need to put negative in this register
            format("ADD R%d, R%d, #1\n", ret, ret); // This needs to be positive
            #endif
            break;
        }
        case OP_NOTEQUALS: {
            //format("DONT DO NOTEQULAS");
            break;
        }

    }
    codegen_state.regfile[ret] = USED;
    return ret;
}

// Sethi-Ullman Register Allocation for the expression rooted at this node.
static int16_t emit_expression_node(ast_node_h node_h) {
    
    struct AST_NODE_STRUCT node = ast_node_data(node_h);

    // TODO: Add immediates.
    if (node.type == A_BINARY_EXPR) {
        struct AST_NODE_STRUCT left = ast_node_data(node.as.expr.binary.left);
        struct AST_NODE_STRUCT right = ast_node_data(node.as.expr.binary.right);
        switch (node.as.expr.binary.type) {
            case OP_ASSIGN: {
                // Need to do lots of checking here, depending on type of lvalue assign does different things in assembly.
                int16_t reg = emit_expression_node(node.as.expr.binary.right);
                codegen_state.regfile[reg] = USED;
                
                // Dereference of an expression
                if (left.type == A_UNARY_EXPR & left.as.expr.unary.type == OP_MUL) {
                    // Treat the child of left as an address, and just store into that.
                    int16_t addr = emit_expression_node(left.as.expr.unary.child);
                    emit_inst_comment((lc3_instruction_t) {.opcode = STR, .arg1 = reg, .arg2 = addr, .arg3 = 0 }, "dereference pointer");
                    codegen_state.regfile[addr] = UNUSED;
                }

                else {
                    symbol_table_entry_t symbol = symbol_table_search(&(analysis.symbol_table), node.as.expr.symbol.token, analysis.scopes.symbol_refs[node.as.expr.binary.left]);
                    // Static Variable:
                    if (symbol.type_info.specifier_info.is_static) {
                        char* var_name = format("%s.%s", codegen_state.current_function_name, symbol.identifier);
                        emit_inst_comment((lc3_instruction_t) {.opcode = ST, .arg1 = reg, .label = var_name}, "assign to static variable");
                        
                    }
                    // Normal Variable
                    else {
                        emit_inst_comment((lc3_instruction_t) {.opcode = STR, .arg1 = reg, .arg2 = 5, .arg3 = -1 * symbol.offset}, format("assign to variable \"%s\"", symbol.identifier));
                        
                    }
                }
                //struct AST_NODE_STRUCT child = ast_node_data(node.as.)
                
                return reg;
            }
            case OP_ADD: {
                int8_t r1 = 0;
                // we can only use one immediate though.
                if (left.type == A_INTEGER_LITERAL && left.as.expr.literal.value <= 15) {
                    // If the leaf is an int literal, we can just use an immediate.
                    r1 = emit_expression_node(node.as.expr.binary.right);
                    int8_t val = left.as.expr.literal.value;
                    emit_inst((lc3_instruction_t) {.opcode = ADDimm, .arg1 = r1, .arg2 = r1, .arg3 = val});
                    codegen_state.regfile[r1] = USED;
                }
                else if (right.type == A_INTEGER_LITERAL && right.as.expr.literal.value <= 15) {
                    r1 = emit_expression_node(node.as.expr.binary.left);
                    int8_t val = right.as.expr.literal.value;
                    emit_inst((lc3_instruction_t) {.opcode = ADDimm, .arg1 = r1, .arg2 = r1, .arg3 = val});
                    codegen_state.regfile[r1] = USED;
                }
                else {
                    r1 = emit_expression_node(node.as.expr.binary.left);
                    codegen_state.regfile[r1] = USED;
                    int8_t r2 = emit_expression_node(node.as.expr.binary.right);
                    codegen_state.regfile[r2] = UNUSED;
                    emit_inst((lc3_instruction_t) {.opcode = ADDreg, .arg1 = r1, .arg2 = r1, .arg3 = r2});
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
                    emit_inst((lc3_instruction_t) {.opcode = ADDimm, .arg1 = r1, .arg2 = r1, .arg3 = val});
                    // TOOD: Only if the immediate is small enough
                    codegen_state.regfile[r1] = USED;
                }
                else {
                    // Otherwise the val is in a register somehow.
                    r1 = emit_expression_node(node.as.expr.binary.left);
                    codegen_state.regfile[r1] = USED;
                    int32_t r2 = emit_expression_node(node.as.expr.binary.right);
                    emit_inst((lc3_instruction_t) {.opcode = NOT, .arg1 = r2, .arg2 = r2});
                    emit_inst((lc3_instruction_t) {.opcode = ADDimm, .arg1 = r2, .arg2 = r2, .arg3 = 1});
                    emit_inst((lc3_instruction_t) {.opcode = ADDreg, .arg1 = r1, .arg2 = r1, .arg3 = r2});
                    codegen_state.regfile[r2] = UNUSED;
                }
            return r1;
            }
            case OP_MUL: {
                link_multiply();
                int32_t r1 = emit_expression_node(node.as.expr.binary.left);
                codegen_state.regfile[r1] = USED;
                int32_t r2 = emit_expression_node(node.as.expr.binary.right);
                codegen_state.regfile[r2] = USED;
                emit_inst((lc3_instruction_t) {.opcode = ST, .arg1 = r1, .label = "MULTIPLY_OP1"});
                emit_inst((lc3_instruction_t) {.opcode = ST, .arg1 = r2, .label = "MULTIPLY_OP2"});
                emit_inst((lc3_instruction_t) {.opcode = JSR, .label = "MULTIPLY"});
                emit_inst((lc3_instruction_t) {.opcode = LD, .arg1 = r1, .label = "MULTIPLY_OUTPUT"});

                
                

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
            /**
            case OP_INCREMENT: {
                if (node.as.expr.unary.order == POSTFIX) {
                    // Scratchpad register
                    uint8_t result = get_empty_reg();
                    // Load variable, add to it, return variable
                    // Need to support things like (*i)++;
                    // Basically need to implement lvalues and rvalues.
                    uint8_t symbol = emit_expression_node(node.as.expr.unary.child);

                }   
                else if (node.as.expr.unary.order == PREFIX) {
                    // Increment, then return
                }
            }
            */
            case OP_ADD: {
                // Litreally doens't do anything, just return 
                return emit_expression_node(node.as.expr.unary.child);
            }
            case OP_SUB: {
                // Get the register that the child expression is in.
                int r = emit_expression_node(node.as.expr.unary.child);
                // Negate it.
                emit_inst((lc3_instruction_t) {.opcode = NOT, .arg1 = r, .arg2 = r});
                emit_inst((lc3_instruction_t) {.opcode = ADDimm, .arg1 = r, .arg2 = r, .arg3 = 1});
                return r; // Its stored in the same register still.
            }
            /** Address of identifier*/
            case OP_BITAND: {
                // Semant should have checked that this is a lvalue -> rvalue
                struct AST_NODE_STRUCT child = ast_node_data(node.as.expr.unary.child);
                ast_node_h child_h = node.as.expr.unary.child;
                // Assert that it is an lvalue
                assert(child.type == A_SYMBOL_REF);
                symbol_table_entry_t symbol = symbol_table_search(&(analysis.symbol_table), child.as.expr.symbol.token, analysis.scopes.symbol_refs[child_h]);
                uint8_t r = get_empty_reg();

                // TODO: Global or Static Symbols (LEA)
                if (symbol.type_info.specifier_info.is_static) {
                    emit_inst_comment((lc3_instruction_t) {.opcode = LEA, .arg1 = r, .label = symbol.identifier}, "load address of static variable");
                }
                else if (symbol.type == PARAMETER_ST_ENTRY) { // Is a parameter
                    // Address of a parameter is just R5 + offset
                    emit_inst_comment((lc3_instruction_t) {.opcode = ADDimm, .arg1 = r, .arg2 = 5, .arg3 = symbol.offset + 4}, format("take address of parameter \"%s\"", symbol.identifier));
                }
                else { // Not a parameter
                    // Address of local variable is just R5 + some other offset
                    emit_inst_comment((lc3_instruction_t) {.opcode = ADDimm, .arg1 = r, .arg2 = 5, .arg3 = -1 * symbol.offset}, format("take address of local variable \"%s\"", symbol.identifier));
                }
                return r;

            }
            /** Dereference*/
            case OP_MUL: {
                u16 r = emit_expression_node(node.as.expr.unary.child);
                // Register contains an address.
                emit_inst_comment((lc3_instruction_t) {.opcode = LDR, .arg1 = r, .arg2 = r, .arg3 = 0}, "dereference register as pointer");
                return r;
            }
        }
    }
    else if (node.type == A_FUNCTION_CALL) {
        // Push arguments right to left.
        for (int i = node.as.expr.call.arguments.size - 1; i >= 0; i--) {
            int r = emit_expression_node(node.as.expr.call.arguments.data[i]);
            // Push r to stack, load next 
            emit_inst((lc3_instruction_t) {.opcode = ADDimm, .arg1 = 6, .arg2 = 6, .arg3 = -1});
            emit_inst_comment((lc3_instruction_t) {.opcode = STR, .arg1 = r, .arg2 = 6, .arg3 = 0}, \
                        "push argument to stack");
            // newline
            codegen_state.regfile[r] = UNUSED;
            emit_newline();
        }
        // TODO: Support calling arbitrary memory locations as functions.
        char* identifier = ast_node_data(node.as.expr.call.symbol_ref).as.expr.symbol.token.contents;
        emit_inst((lc3_instruction_t) {.opcode = JSR, .label = identifier});

        // Handle Return Value:
        int ret = get_empty_reg();
        emit_inst_comment((lc3_instruction_t) {.opcode = LDR, .arg1 = ret, .arg2 = 6, .arg3 = 0}, \
                        "load return value");
        emit_inst_comment((lc3_instruction_t) {.opcode = ADDimm, .arg1 = 6, .arg2 = 6, .arg3 = 1}, \
                        "pop return value");
        emit_inst_comment((lc3_instruction_t) {.opcode = ADDimm, .arg1 = 6, .arg2 = 6, .arg3 = node.as.expr.call.arguments.size}, \
                        "pop arguments");
        return ret;
    }
    else if (node.type == A_INTEGER_LITERAL) {
        // We probably just want to place this in a constant pool and load from there.
        // Get a register to place this in
        int val = node.as.expr.literal.value;
        // TODO: If val is > 15 then we have to materialize this int somehow.
        int r1 = get_empty_reg();
        emit_inst((lc3_instruction_t) {.opcode = ANDimm, .arg1 = r1, .arg2 = r1, .arg3 = 0});
        if (val != 0) {
            emit_inst((lc3_instruction_t) {.opcode = ADDimm, .arg1 = r1, .arg2 = r1, .arg3 = val});
        }
        codegen_state.regfile[r1] = USED;
        return r1;
    }
    else if (node.type == A_SYMBOL_REF) {
        // Do now
        symbol_table_entry_t symbol = symbol_table_search(&(analysis.symbol_table), node.as.expr.symbol.token, analysis.scopes.symbol_refs[node_h]);
        // Load 
        // Parameters need a positive offset??
        // Is a parameter
        int r1 = get_empty_reg();
        if (symbol.type_info.specifier_info.is_static) {
            emit_inst_comment((lc3_instruction_t) {.opcode = LD, .arg1 = r1, .label = symbol.identifier}, \
                        format("load static variable", symbol.identifier));
        }
        else if (symbol.type == PARAMETER_ST_ENTRY) { // Is a parameter
            emit_inst_comment((lc3_instruction_t) {.opcode = LDR, .arg1 = r1, .arg2 = 5, .arg3 = symbol.offset + 4}, \
                        format("load parameter \"%s\"", symbol.identifier));
        }
        else { // Not a parameter
        emit_inst_comment((lc3_instruction_t) {.opcode = LDR, .arg1 = r1, .arg2 = 5, .arg3 = -1 * symbol.offset}, \
                        format("load local variable \"%s\"", symbol.identifier));
        }
        return r1;
    }
    printf(ANSI_COLOR_RED "error: " ANSI_COLOR_RESET "Unsupported C feature encountered, please rewrite your code to not use this feature.\n");
    print_ast_node(node_h, 2);
}

void emit_ast_node(ast_node_h node_h) {
    static uint16_t if_counter = 0;
    static uint16_t while_counter = 0;
    static uint16_t for_counter = 0;
    if (node_h == -1) {
        return;
    }

    struct AST_NODE_STRUCT node = ast_node_data(node_h);

    codegen_state.regfile[0] = UNUSED;
    codegen_state.regfile[1] = UNUSED;
    codegen_state.regfile[2] = UNUSED;
    codegen_state.regfile[3] = UNUSED;

    switch (node.type) {
        case A_PROGRAM: {
            for (int i = 0; i < (node.as.program.body.size); i++)
                emit_ast_node(node.as.program.body.data[i]);
            //format("HALT\n");
            return;
        }
        // Somehow we need to pattern match these two into the same instruction.
        // Binops on immediates can be one instruction if the immediate is small enough.
        // How does one even do that?
        case A_RETURN_STMT: {
            // Load the child into R0
            // Check if its already in R0???
            // No optimizations yet.
            bool is_main = !strcmp(codegen_state.current_function_name, "main");

            // Not main
            if (!is_main) {
                if (node.as.stmt._return.expression != -1) {
                    int16_t reg = emit_expression_node(node.as.stmt._return.expression);
                    // Write it into return value slot, which 
                    emit_inst_comment((lc3_instruction_t) {.opcode = STR, .arg1 = reg, .arg2 = 5, .arg3 = 3}, \
                            "write return value, always R5 + 3");
                }
                // Need to know what funciton we are returning from somehow.
                char* teardown_label = format("%s.teardown", codegen_state.current_function_name);
                emit_inst((lc3_instruction_t) {.opcode = BR, .arg1 = 1, .arg2 = 1, .arg3 = 1, .label = teardown_label});
            }
            else { 
                // Write reg to return slot
                if ((node.as.stmt._return.expression != -1)) {
                    int16_t reg = emit_expression_node(node.as.stmt._return.expression);
                    emit_inst_comment((lc3_instruction_t) {.opcode = STI, .arg1 = reg, .label = "RETURN_SLOT"}, \
                            "write return value from main");
                }
                emit_inst((lc3_instruction_t) {.opcode = HALT});
            }
            
            return;
        }
        case A_WHILE_STMT: {
            // Todo: Indent everything in the loop
            char* loop_header_name = format("%s.while.%d", codegen_state.current_function_name, while_counter);
            char* loop_end_name  = format("%s.while.%d.end", codegen_state.current_function_name, while_counter);

            emit_label(loop_header_name);
            int32_t r_condition = emit_expression_node(node.as.stmt._while.condition);
            
            emit_inst_comment((lc3_instruction_t) {.opcode = ANDreg, .arg1 = r_condition, .arg2 = r_condition, .arg3 = r_condition}, \
                        "load condition into NZP");

            
            emit_inst_comment((lc3_instruction_t) {.opcode = BR, .arg1 = 1, .arg2 = 1, .arg3 = 0, .label = loop_end_name}, \
                        "if false, skip over loop body");

            emit_ast_node(node.as.stmt._while.body);

            emit_inst_comment((lc3_instruction_t) {.opcode = BR, .arg1 = 1, .arg2 = 1, .arg3 = 1, .label = loop_header_name}, \
                        "test loop condition");

            emit_label(loop_end_name);
            while_counter++;
            return;
        }
        case A_FOR_STMT: {
            emit_comment("for loop initiailization");

            emit_ast_node(node.as.stmt._for.initilization);

            char* loop_header_name = format("%s.for.%d", codegen_state.current_function_name, for_counter);
            char* loop_end_name  = format("%s.for.%d.end", codegen_state.current_function_name, for_counter);

            emit_label(loop_header_name);

            emit_comment("test condition");
            int32_t r_condition = emit_expression_node(node.as.stmt._for.condition);
            emit_newline();

            emit_inst_comment((lc3_instruction_t) {.opcode = ANDreg, .arg1 = r_condition, .arg2 = r_condition, .arg3 = r_condition}, \
                        "load condition into NZP");
            
            emit_inst_comment((lc3_instruction_t) {.opcode = BR, .arg1 = 1, .arg2 = 1, .arg3 = 0, .label = loop_end_name}, \
                        "if false, skip over loop body");

            emit_ast_node(node.as.stmt._for.body);

            emit_comment("increment expression");
            emit_ast_node(node.as.stmt._for.update);

            emit_inst_comment((lc3_instruction_t) {.opcode = BR, .arg1 = 1, .arg2 = 1, .arg3 = 1, .label = loop_header_name}, \
                        "test loop condition again");

            emit_label(loop_end_name);

            for_counter++;
            return;
        }
        case A_IF_STMT: {
            int16_t r_condition = emit_expression_node(node.as.stmt._if.condition);
            emit_newline();
            emit_inst_comment((lc3_instruction_t) {.opcode = ANDreg, .arg1 = r_condition, .arg2 = r_condition, .arg3 = r_condition}, \
                        "load condition into NZP");

            char* if_codegen_statement_end  = format("%s.if.%d.end", codegen_state.current_function_name, if_counter);
            char* else_codegen_statement_name  = format("%s.if.%d", codegen_state.current_function_name, if_counter);

            // Else Statement:
            if (node.as.stmt._if.else_stmt != -1) {
                emit_inst_comment((lc3_instruction_t) {.opcode = BR, .arg1 = 1, .arg2 = 1, .arg3 = 0, .label = else_codegen_statement_name}, \
                        "if false, jump to else codegen_statement");

                emit_newline();
                emit_ast_node(node.as.stmt._if.if_stmt);

                emit_inst((lc3_instruction_t) {.opcode = BR, .arg1 = 1, .arg2 = 1, .arg3 = 0, .label = if_codegen_statement_end});
                
                emit_label(else_codegen_statement_name);

                emit_newline();
                emit_ast_node(node.as.stmt._if.else_stmt);

                emit_label(if_codegen_statement_end);

            } // No Else Statement:
            else {
                emit_inst_comment((lc3_instruction_t) {.opcode = BR, .arg1 = 1, .arg2 = 1, .arg3 = 0, .label = if_codegen_statement_end}, \
                        "if false, jump over codegen_statement");
                emit_newline();
                emit_ast_node(node.as.stmt._if.if_stmt);
                
                emit_label(if_codegen_statement_end);
            }
            if_counter++;
            
            return;
        }
        case A_COMPOUND_STMT: {
            for (int i = 0; i < (node.as.stmt.compound.statements.size); i++) {
                emit_ast_node(node.as.stmt.compound.statements.data[i]);
                emit_newline();
            }
                
            return;
        }
        case A_FUNCTION_DECL: {
            bool is_main = !strcmp(node.as.func_decl.token.contents, "main");
            codegen_state.current_function_name = node.as.func_decl.token.contents;

            emit_label(codegen_state.current_function_name);

            if (!is_main) {
                emit_comment("callee setup:");

                // TODO: Enable condensed callee setup. 
                emit_inst_comment((lc3_instruction_t) {.opcode = ADDimm, .arg1 = 6, .arg2 = 6, .arg3 = -1}, \
                            "allocate spot for return value");

                emit_inst((lc3_instruction_t) {.opcode = ADDimm, .arg1 = 6, .arg2 = 6, .arg3 = -1});

                emit_inst_comment((lc3_instruction_t) {.opcode = STR, .arg1 = 7, .arg2 = 6, .arg3 = 0}, \
                            "push R7 (return address)");

                emit_inst((lc3_instruction_t) {.opcode = ADDimm, .arg1 = 6, .arg2 = 6, .arg3 = -1});

                emit_inst_comment((lc3_instruction_t) {.opcode = STR, .arg1 = 5, .arg2 = 6, .arg3 = 0}, \
                            "push R5 (caller frame pointer)");

                emit_inst_comment((lc3_instruction_t) {.opcode = ADDimm, .arg1 = 5, .arg2 = 6, .arg3 = -1}, \
                            "set frame pointer");
                
                emit_newline();

                emit_comment("function body:");
                emit_ast_node(node.as.func_decl.body);

                char* teardown_label = format("%s.teardown", codegen_state.current_function_name);
                emit_label(teardown_label);
                
                emit_inst_comment((lc3_instruction_t) {.opcode = ADDimm, .arg1 = 6, .arg2 = 5, .arg3 = 1}, \
                            "pop local variables");
                
                emit_inst_comment((lc3_instruction_t) {.opcode = LDR, .arg1 = 5, .arg2 = 6, .arg3 = 0}, \
                            "pop frame pointer");

                emit_inst((lc3_instruction_t) {.opcode = ADDimm, .arg1 = 6, .arg2 = 6, .arg3 = 1});

                emit_inst_comment((lc3_instruction_t) {.opcode = LDR, .arg1 = 7, .arg2 = 6, .arg3 = 0}, \
                            "pop return address");

                emit_inst((lc3_instruction_t) {.opcode = ADDimm, .arg1 = 6, .arg2 = 6, .arg3 = 1});
                emit_inst((lc3_instruction_t) {.opcode = RET});
                emit_comment("end function");
                emit_newline();
            }

            else if (is_main) {
                emit_ast_node(node.as.func_decl.body);
                //emit_inst((lc3_instruction_t) {.opcode = HALT});
            }
            return;
        }
        case A_PARAM_DECL: 
            // Don't do anything.
            return;
        case A_VAR_DECL: {
            // Is global scope
            // We will have a global data section, instead of placing them in the same order
            // as they are declared in the program.
            // Global data section is at address x5000
            // Global variable
            // This decl needs to point to a 
            if (analysis.scopes.var_decls[node_h] == 0) { 
                uint16_t init_value = 0;
                if (node.as.var_decl.initializer != -1) {
                    init_value = ast_node_data(node.as.var_decl.initializer).as.expr.literal.value;
                } 
                emit_data(node.as.var_decl.token.contents, (lc3_directive_t) {.type = FILL, .value = init_value});
                return;
            }
            
            else if (node.as.var_decl.type_info.specifier_info.is_static) {
                // Static Local variable
                char* identifier = format("%s.%s", codegen_state.current_function_name, node.as.var_decl.token.contents);
                // TODO: EMPHASIZE the fact that initial value of static vars in functions are implementation defined.
                if (node.as.var_decl.initializer != -1) {
                    uint16_t init_value = ast_node_data(node.as.var_decl.initializer).as.expr.literal.value;
                    emit_data(identifier, (lc3_directive_t) {.type = FILL, .value = init_value});
                }
                else {
                    emit_data(identifier, (lc3_directive_t) {.type = FILL, .value = 0xECEB});
                }
                return;
            }
            else {
                // Nonstatic Local Variable, ideally we should insert this 
                // allocate space for local variable
                emit_inst_comment((lc3_instruction_t) {.opcode = ADDimm, .arg1 = 6, .arg2 = 6, .arg3 = -1}, \
                        format("Allocate space for \"%s\"", node.as.var_decl.token.contents));

                if (node.as.var_decl.initializer != -1) {
                    // TOOD: Assignment nodes should not really be separate, there should be an assignemnt expression.
                    // But for now, we can kepe initialization separate, as for static ints it works different I suppose
                    // Load a reg with the initializer value
                    int reg = emit_expression_node(node.as.var_decl.initializer);
                    symbol_table_entry_t symbol = symbol_table_search(&(analysis.symbol_table), node.as.var_decl.token, analysis.scopes.var_decls[node_h]);

                    emit_inst_comment((lc3_instruction_t) {.opcode = STR, .arg1 = reg, .arg2 = 5, .arg3 = -1 * symbol.offset}, \
                            format("Initialize \"%s\"", symbol.identifier));

                    codegen_state.regfile[reg] = UNUSED;
                }
                // newline?
                return;
            }
            
        }
        case A_INLINE_ASM: {
            static char inline_asm_buffer[128];
            for (int i = 0; i < 128; i++) {
                inline_asm_buffer[i] = 0;
            }
            // Copy contents to buffer, ignoring backslashes.
            int j = 0;
            for (int i = 0; i < 128; i++) {
                char c = node.as.stmt.inline_asm.token.contents[i];
                if (c == '\0') {
                    break;
                }
                
                if (c == '\\') {
                    continue;
                }
                inline_asm_buffer[j++] = c;
            }
            emit_label(inline_asm_buffer);
            return;
        }
            
        case A_FUNCTION_CALL:
        case A_UNARY_EXPR:
        case A_BINARY_EXPR: 
        case A_INTEGER_LITERAL: 
        case A_SYMBOL_REF: 
            emit_expression_node(node_h);
            return;
        
    }
}
