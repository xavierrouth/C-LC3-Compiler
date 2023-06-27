#include "symbol_table.h"

symbol_table_t* symbol_table_init(symbol_table_t* parent) {
    symbol_table_t* table = malloc(sizeof(*table));
    table->symbols = symbol_vector_init(16);
    table->children = symbol_table_vector_init(2);
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
