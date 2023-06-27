#include "symbol_table.h"
#include <string.h>

symbol_table_t* symbol_table_init(symbol_table_t* parent) {
    symbol_table_t* table = malloc(sizeof(*table));
    table->symbols = symbol_vector_init(16);
    table->children = symbol_table_vector_init(2);
    table->offset = 0;
    table->parent = parent;
    return table;
}

void symbol_table_free(symbol_table_t* table) {
    for (int i = 0; i < table->children.size; i++) {
        symbol_table_free(table->children.data[i]);
    }
    free(table);
    return;
}

symbol_table_entry* symbol_table_search(symbol_table_t* branch, char* identifier) {
    if (branch == NULL) {
        return NULL;
    }

    for (int i = 0; i < branch->symbols.size; i++) {
        if (!strcmp(identifier, branch->symbols.data[i].identifier)) {
            return &(branch->symbols.data[i]);
        }
    }
    // Recurse up, return the first match.
    return symbol_table_search(branch->parent, identifier);
}