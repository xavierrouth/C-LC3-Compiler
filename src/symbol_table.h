#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string.h>
#include <stdbool.h>
#include "types.h"

typedef struct SYMBOL_TABLE_ENTRY_STRUCT {
    char* identifier;
    type_info_t type_info;
    int stack_offset; //Location on stack frame as offset
    bool parameter;
    // These should all be negative as stack grows down.
} symbol_table_entry;

#define T symbol_table_entry
#define NAME symbol
#include "vector.h"
#undef NAME
#undef T

typedef struct SYMBOL_TABLE_STRUCT symbol_table_t;

#define T symbol_table_t*
#define NAME symbol_table
#include "vector.h"
#undef NAME
#undef T

struct SYMBOL_TABLE_STRUCT {
    char* name; // Global / main / etc...
    symbol_vector symbols;
    symbol_table_t* parent;
    int offset;
};

symbol_table_t* symbol_table_init(symbol_table_t* parent); 

void symbol_table_free(symbol_table_t* table);

symbol_table_entry* symbol_table_search(symbol_table_t* branch, char* identifier);

#endif
