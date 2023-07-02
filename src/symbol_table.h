#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string.h>
#include <stdbool.h>
#include "types.h"

typedef struct SYMBOL_TABLE_ENTRY_STRUCT {
    char* identifier;
    type_info_t type_info;
    int size; 
    int offset; //Location on stack frame as offset
    enum {
        PARAMETER,
        VARIABLE,
        FUNCTION
    } type;
    int table; // Index into the symbol table (metadata, can find vars of same scope).
    // These should all be negative as stack grows down.
} symtable_entry;

#define T symtable_entry
#define NAME symbol
#include "vector.h"
#undef NAME
#undef T

typedef struct SYMBOL_SUBTABLE_STRUCT {
    char* name; // Global / main / etc...
    symbol_vector symbols;
    int parent;
    int offset;
} symtable_t;

#define T symtable_t
#define NAME symtable
#include "vector.h"
#undef NAME
#undef T

extern symtable_vector symtable_root;

void symtable_root_init(); 

int symtable_get_parent(int scope);

void symtable_root_free();

// Initialize a subtable
int symtable_branch_init(int parent, char* name); 

symtable_entry* symtable_search(int branch, char* identifier);

#endif
