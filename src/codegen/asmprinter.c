
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "asmprinter.h"
#include "memory/bump_allocator.h"

#define PRINTER_ALLOCATOR_SIZE 2048 * 16
#define COMMENT_POSITION 30
#define DIRECTIVE_POSITION 20
#define INDENT 4

static u8 buffer[PRINTER_ALLOCATOR_SIZE];

static bump_allocator_t printer_allocator = {
    .start = &buffer[0],
    .end = &buffer[PRINTER_ALLOCATOR_SIZE - 1],
    .ptr = &buffer[PRINTER_ALLOCATOR_SIZE - 1]
};

static asm_printer_state_t printer_state;

void init_asmprinter() {
    printer_state.root = init_block();

}

// Unclear if this should be part of the printer state.
static FILE* outfile;

void set_out_file(char* path) {
    outfile = fopen(path, "w");
    return;
}

void close_out_file(void) {
    fclose(outfile);
}

static const char* asm_format(const char* fmt, ...) {
    va_list args_const;
    va_start(args_const, fmt);

    va_list args_use;
    va_copy(args_use, args_const);

    /**
    vprintf(fmt, args_use);
    va_copy(args_use, args_const);
    */

    uint32_t length = vsnprintf(NULL, 0, fmt, args_use) + 1;
    va_copy(args_use, args_const);
    char* buff = bump_allocate(&printer_allocator, sizeof(char), sizeof(char) * length);    
    vsnprintf(buff, length, fmt, args_use);

    //printf(buff);

    va_end(args_use);
    va_end(args_const);
    return buff;
}

static const char* asm_format_directive(lc3_directive_t* directive){
    switch (directive->type) {
        case FILL:
            return asm_format(".FILL x%04X", directive->value);
        default:
            return "rjhg3ekrjlg";
    }
}

static const char* asm_format_inst(lc3_instruction_t* inst) {
    switch (inst->opcode) {
        case NOOP:
            return asm_format("\n");
        case ADDreg:
            return asm_format("ADD R%u, R%u, R%d", inst->arg1, inst->arg2, inst->arg3);
        case ADDimm:
            return asm_format("ADD R%u, R%u, #%d", inst->arg1, inst->arg2, inst->arg3);
        case ANDreg:
            return asm_format("AND R%u, R%u, R%d", inst->arg1, inst->arg2, inst->arg3);
        case ANDimm:
            return asm_format("AND R%u, R%u, #%d", inst->arg1, inst->arg2, inst->arg3);
        case BR: {
            // Uncoditional branch
            if (inst->arg1 && inst->arg2 && inst->arg3) {
                return asm_format("BR %s", inst->label);
            }
            if (inst->arg1 && inst->arg2) {
                return asm_format("BRnz %s", inst->label);
            }
            if (inst->arg1 && inst->arg3) {
                return asm_format("BRnp %s", inst->label);
            }
            if (inst->arg2 && inst->arg3) {
                return asm_format("BRzp %s", inst->label);
            }
            if (inst->arg1) {
                return asm_format("BRn %s", inst->label);
            }
            if (inst->arg2) {
                return asm_format("BRz %s", inst->label);
            }
            if (inst->arg3) {
                return asm_format("BRp %s", inst->label);
            }
        }
        case JSR:
            return asm_format("JSR %s", inst->label);
        case JSRR:
            return "";
        case LD:
            return asm_format("LD R%d, %s", inst->arg1, inst->label);
        case LDI:
            return "";
        case LDR:
            return asm_format("LDR R%d, R%d, #%d", inst->arg1, inst->arg2, inst->arg3);
        case NOT:
            return asm_format("NOT R%u, R%u", inst->arg1, inst->arg2);
        case RET:
            return "RET";
        case RTI:
            return "erwat";
        case ST:
            return asm_format("ST R%d, %s", inst->arg1, inst->label);
        case STI:
            return asm_format("STI R%d, %s", inst->arg1, inst->label);
        case STR:
            return asm_format("STR R%d, R%d, #%d", inst->arg1, inst->arg2, inst->arg3);
        case HALT:
            return "HALT";
        default:
            return "203894024";
    }
}


static int write(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = vfprintf(outfile, fmt, args);
    va_end(args);
    return len;
}

static void write_bundle(asm_bundle_t* bundle) {
    // Todo: cleanup.
   
    // Print bundle as so:
    // LABEL
    //     AND R2,R2,#0      ; number of negative operands
    //     ADD R1,R1,#0	     ; set R1 to its negative absolute value
    //
    // DATA   .FILL x0000
    // DATA   .STRINGZ "Hello World"

    
    // Inline assembly:
    // Todo: Format this??
    /**
    if (bundle->is_inline_asm) {
        write(bundle->label);
        return;
    }
    */

    int linepos = 0;
    // Write label
    if (bundle->label != NULL) {
        linepos += write(bundle->label);
    }
    // Write instruction
    switch(bundle->type) {
        case B_INSTRUCTION: {
            while(linepos < INDENT) {
                linepos += write("%*s", INDENT-linepos, " ");
            }
            linepos += write(asm_format_inst(&(bundle->instruction)));
        }
        case B_DIRECTIVE: {
            while(linepos < DIRECTIVE_POSITION) {
                linepos += write("%*s", INDENT-linepos, " ");
            }
            linepos += write(asm_format_directive(&(bundle->directive)));
        }
    }
    // Write comment
    if (bundle->comment != NULL){
        // Inline comment
        if (linepos != 0) {
            while(linepos < COMMENT_POSITION) {
                linepos += write("%*s", COMMENT_POSITION-linepos, " ");
            }
        }
        
        write("; ");
        write((bundle->comment));
    }

    write("\n");

    return;
}

static void write_block(asm_block_t* block) {
    if (block->header)
        write("%s", block->header);

    asm_bundle_t* instruction = block->instructions_head;
    while (instruction != NULL) {
        write_bundle(instruction);
        instruction = instruction->next;
    }
    write("\n");
    write("; ---- Data Section ----\n");
    asm_bundle_t* data = block->instructions_head;
    while (data != NULL) {
        write_bundle(data);
        data = data->next;
    }
    if (block->footer)
        write("%s", block->footer);
}

static void write_program_header() {
    write("; ------------------------------------------\n; C-LC3 Compiler, By HKN for UIUC Students\n; ------------------------------------------\n");
    write("\n");
    write(".ORIG x3000\nLD R6, USER_STACK\nADD R5, R6, #-1\nJSR main\n");
    write("\n");
}

static void write_program_footer() {
    write("\n");
    write("USER_STACK .FILL xFDFF\nRETURN_SLOT .FILL xFDFF\n");
    write("\n");
}

bool multiplication_dependency;

void link_multiply() {
    multiplication_dependency = true;
}
// Links subroutine

#include "multiply.h"

void write_to_file(char* path) {
    set_out_file(path);
    write_program_header();
    
    //todo Loop:
    write_block(printer_state.root);

    write_program_footer();
    if (multiplication_dependency) {
        write(multiply_asm);
    }
    write(".END\n");
    close_out_file();
}

asm_bundle_t* init_bundle(void) {
    asm_bundle_t* bundle = bump_allocate(&printer_allocator, sizeof(*bundle), sizeof(*bundle));
    return bundle;
}

asm_block_t* init_block(void) {
    asm_block_t* block = bump_allocate(&printer_allocator, sizeof(*block), sizeof(*block));
    *block = (asm_block_t) {
        .footer = "\0",
        .header = "\0",
        .next = NULL,
        .instructions_head = NULL,
        .instructions_tail = NULL,
        .data_head = NULL,
        .data_tail = NULL
    };
    return block;
}

/**
void emit_data(char* label, char* directive, asm_block_t* block) {
    block->data[block->data_size++] = init_instruction(NULL, label, directive, NULL);
}


void emit_directive(char* directive, asm_block_t* block) {
    block->instructions[block->instructions_size++] = init_instruction(NULL, NULL, directive, NULL);
}
*/

void emit_newline(void) {
    asm_bundle_t* bundle = init_bundle();
    if (printer_state.root->instructions_tail == NULL) {
        printer_state.root->instructions_head = bundle;
        printer_state.root->instructions_tail = bundle;
        return;
    }
    printer_state.root->instructions_tail->next = bundle;
    printer_state.root->instructions_tail = bundle;
}

void emit_comment(char* comment) {
    asm_bundle_t* bundle = init_bundle();
    *bundle = (asm_bundle_t) {.comment = comment};
    if (printer_state.root->instructions_tail == NULL) {
        printer_state.root->instructions_head = bundle;
        printer_state.root->instructions_tail = bundle;
        return;
    }
    printer_state.root->instructions_tail->next = bundle;
    printer_state.root->instructions_tail = bundle;
}
void emit_label(char* label) {
    asm_bundle_t* bundle = init_bundle();
    *bundle = (asm_bundle_t) {.label = label};
    if (printer_state.root->instructions_tail == NULL) {
        printer_state.root->instructions_head = bundle;
        printer_state.root->instructions_tail = bundle;
        return;
    }
    printer_state.root->instructions_tail->next = bundle;
    printer_state.root->instructions_tail = bundle;
}

void emit_data(char* label, lc3_directive_t directive) {
    asm_bundle_t* bundle = init_bundle();
    *bundle = (asm_bundle_t) {{.directive = directive}, .label = label, .type = B_DIRECTIVE};
    if (printer_state.root->data_tail == NULL) {
        printer_state.root->data_head = bundle;
        printer_state.root->data_tail = bundle;
        return;
    }
    printer_state.root->data_tail->next = bundle;
    printer_state.root->data_tail = bundle;
}

void emit_inst(lc3_instruction_t inst) {
    asm_bundle_t* bundle = init_bundle();
    *bundle = (asm_bundle_t) {{.instruction = inst}, .type = B_INSTRUCTION};
    if (printer_state.root->instructions_tail == NULL) {
        printer_state.root->instructions_head = bundle;
        printer_state.root->instructions_tail = bundle;
        return;
    }
    printer_state.root->instructions_tail->next = bundle;
    printer_state.root->instructions_tail = bundle;
}

void emit_inst_comment(lc3_instruction_t inst, char* comment) {
    asm_bundle_t* bundle = init_bundle();
    *bundle = (asm_bundle_t) {{.instruction = inst}, .type = B_INSTRUCTION, .comment = comment};
    if (printer_state.root->instructions_tail == NULL) {
        printer_state.root->instructions_head = bundle;
        printer_state.root->instructions_tail = bundle;
        return;
    }
    printer_state.root->instructions_tail->next = bundle;
    printer_state.root->instructions_tail = bundle;
}



