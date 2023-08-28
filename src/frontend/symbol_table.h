#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include "token.h"

#define SYMBOL_TABLE_MAX_SIZE 100

typedef struct SYMBOL_TABLE_ENTRY_STRUCT {
    char* identifier;
    type_info_t type_info;
    i32 size; 
    i32 offset; //Location on stack frame as offset
    enum {
        PARAMETER_ST_ENTRY,
        VARIABLE_ST_ENTRY,
        FUNCTION_ST_ENTRY
    } type;
    i32 scope;
} symbol_table_entry_t;

typedef struct SYMBOL_TABLE_STRUCT {
    symbol_table_entry_t data[SYMBOL_TABLE_MAX_SIZE];
    uint16_t parent_scope[SYMBOL_TABLE_MAX_SIZE];
    uint16_t idx;
} symbol_table_t;

// Symbol table could just be global?
void symbol_table_add(symbol_table_t* symbol_table, token_t identifier_token, i32 scope, type_info_t return_type, i32 type, i32 size, i32 offset);

symbol_table_entry_t symbol_table_search(symbol_table_t* symbol_table, token_t identifier_token, i32 scope);

#endif
