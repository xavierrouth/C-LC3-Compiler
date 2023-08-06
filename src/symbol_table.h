#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "types.h"

#define SYMBOL_TABLE_MAX_SIZE 100

typedef struct SYMBOL_TABLE_ENTRY_STRUCT {
    char* identifier;
    type_info_t type_info;
    int32_t size; 
    int32_t offset; //Location on stack frame as offset
    enum {
        PARAMETER_ST_ENTRY,
        VARIABLE_ST_ENTRY,
        FUNCTION_ST_ENTRY
    } type;
    int32_t scope;
} symbol_table_entry_t;

extern int32_t parent_scope[SYMBOL_TABLE_MAX_SIZE];

void symbol_table_add(char* identifier, type_info_t return_type, int32_t type, int32_t size, int32_t offset, int32_t scope);
symbol_table_entry_t symbol_table_search(int32_t scope, char* identifier);

#endif
