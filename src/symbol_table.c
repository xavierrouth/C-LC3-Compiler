#include "symbol_table.h"
#include <string.h>

// Array implementation of parent-pointer tree.
// This holds the memory allocated for our symbol table.
symtable_vector symtable_root;

int symtable_branch_init(int parent, char* name) {
    symtable_t table;
    table.symbols = symbol_vector_init(16);
    table.offset = 0;
    table.parent = parent;
    symtable_vector_push(&symtable_root, table);
    return symtable_root.size - 1;
}

void symtable_root_init() {
    // Initialize the root symbol table.
    symtable_branch_init(-1, "global_table"); // Should return 0;
    return;
}

void symtable_root_free() {
    for (int i = 0; i < symtable_root.size; i++) {
        symbol_vector_free(symtable_root.data[i].symbols);
    }
    symtable_vector_free(symtable_root);
}

int symtable_get_parent(int scope) {
    return symtable_root.data[scope].parent;
}

symtable_entry* symtable_search(int branch, char* identifier) {
    for (int i = 0; i < symtable_root.data[branch].symbols.size; i++) {
        if (!strcmp(identifier, symtable_root.data[branch].symbols.data[i].identifier)) {
            return &(symtable_root.data[branch].symbols.data[i]);
        }
    }
    // Recurse up, return the first match.
    return symtable_search(symtable_root.data[branch].parent, identifier);
}