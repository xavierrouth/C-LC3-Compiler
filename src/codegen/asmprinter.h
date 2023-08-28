#ifndef ASMFILE_H
#define ASMFILE_H

#include <stdbool.h>

#include "util/util.h"

// Todo: Clean up allocation

typedef struct {
    enum LC3_OPCODE{
        NOOP,
        ADDreg,
        ADDimm,
        ANDreg,
        ANDimm,
        BR,
        JMP,
        JSR,
        JSRR,
        LD,
        LDI,
        LDR,
        LEA,
        NOT,
        RET,
        RTI,
        ST,
        STI,
        STR,
        GETC,
        OUT,
        PUTS,
        IN,
        PUTSP,
        HALT
    } opcode;
    char* label;
    u8 arg1;
    u8 arg2;
    i16 arg3;
} lc3_instruction_t;

typedef struct {
    enum {
        FILL,
        STRINGZ,
        BLKW,
        ORIG,
        END,
    } type;
    i16 value;
} lc3_directive_t;

typedef struct ASM_BUNDLE_STRUCT { 
    union {
        lc3_instruction_t instruction;
        lc3_directive_t directive;
    };
    enum {
        B_INSTRUCTION,
        B_DIRECTIVE,
        B_INLINE_ASM
    } type;
    char* label;
    char* comment;
    //struct ASM_BUNDLE_STRUCT* previous;
    struct ASM_BUNDLE_STRUCT* next;
} asm_bundle_t;

typedef struct ASM_BLOCK {
    // Linked Lists:
    asm_bundle_t* instructions_head;
    asm_bundle_t* instructions_tail;
    asm_bundle_t* data_head;
    asm_bundle_t* data_tail;
    char* header;
    char* footer;
    struct ASM_BLOCK* next;
} asm_block_t;

typedef struct {
    asm_block_t* root;
} asm_printer_state_t;

void write_to_file(char* path);

asm_block_t* init_block(void);

void emit_data(char* label, lc3_directive_t directive);

void emit_label(char* label);

void emit_directive(char* directive);

void emit_inst(lc3_instruction_t inst);

void emit_comment(char* comment);

void emit_inst_comment(lc3_instruction_t inst, char* comment);

void emit_newline(void);

void link_multiply();

void init_asmprinter();

#endif
