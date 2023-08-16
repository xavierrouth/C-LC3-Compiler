#ifndef ASMFILE_H
#define ASMFILE_H

#include <stdbool.h>

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
    uint8_t arg1;
    uint8_t arg2;
    int16_t arg3;
} lc3_instruction_t;

typedef struct {
    enum {
        FILL,
        STRINGZ,
        BLKW,
        ORIG,
        END,
    } type;
    uint16_t value;
} lc3_directive_t;

typedef struct { 
    union {
        lc3_instruction_t instruction;
        lc3_directive_t directive;
    };
    bool is_inst;
    bool is_directive;
    bool is_inline_asm;
    char* label;
    char* comment;
} asm_bundle_t;

typedef struct ASM_BLOCK {
    asm_bundle_t* instructions[64];
    asm_bundle_t* data[16];
    uint16_t instructions_size;
    uint16_t data_size;
    char* header;
    char* footer;
    struct ASM_BLOCK* next;
} asm_block_t;

typedef struct {
    asm_block_t root;
} asm_printer_state_t;

void write_to_file(char* path, asm_block_t* root);

asm_block_t* init_block(void);

void emit_data(char* label, lc3_directive_t directive, asm_block_t* block);

void emit_label(char* label, asm_block_t* block);

void emit_directive(char* directive, asm_block_t* block);

void emit_inst(lc3_instruction_t inst, asm_block_t* block);

void emit_comment(char* comment, asm_block_t* block);

void emit_inst_comment(lc3_instruction_t inst, char* comment, asm_block_t* block);

void emit_newline(asm_block_t* block);

void link_multiply();

#endif
