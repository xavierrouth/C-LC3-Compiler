#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <stdbool.h>

#include "AST.h"
#include "types.h"



// For now lets just do a global symbol table.
typedef struct SYMBOL_TABLE_ENTRY_STRUCT {
    char identifier[16];
    type_info_t type_info;
    int stack_offset; //Location on stack frame as offset
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
    char name[16]; // Global / main / etc...
    symbol_vector symbols;
    symbol_table_t* parent;
    symbol_table_vector children; // This is a vector of symbol table types.
    int offset;
};

symbol_table_t* symbol_table_init(symbol_table_t* parent); 

void symbol_table_free(symbol_table_t* table);

char* type_info_to_str(type_info_t type_info);

void analysis(ast_node_t* root);

#endif
