#include <string.h>
#include <stdio.h>

#include "symbol_table.h"


symbol_table_entry_t symbol_table[SYMBOL_TABLE_MAX_SIZE];
bool symbol_table_live[SYMBOL_TABLE_MAX_SIZE];

// Parent pointers:
int32_t parent_scope[SYMBOL_TABLE_MAX_SIZE];

symbol_table_entry_t symbol_table_search(int32_t scope, char* identifier) {
    for (int i = 0; i < SYMBOL_TABLE_MAX_SIZE; i++) {
        // Match characterisitcs from easiest to hardest
        if (!symbol_table_live[i]) {
            continue;
        }
        symbol_table_entry_t entry = symbol_table[i];
        if (scope != entry.scope)
            continue;

        if (!strcmp(identifier, entry.identifier)) {
            return entry;
        }
    }
    // Recurse up, return the first match.
    if (scope != 0) {
        // Parent pointer.
        return symbol_table_search(parent_scope[scope], identifier);
    } 
    symbol_table_entry_t invalid = {0};
    // TODO: Error:
    printf("Could not find identifier.\n");
    return invalid;
}

void symbol_table_add(char* identifier, type_info_t return_type, int32_t type, int32_t size, int32_t offset, int32_t scope) {
    static int32_t idx = 0;
    
    symbol_table_entry_t new_entry = {
        .identifier = identifier, 
        .type_info = return_type,
        .type = type,
        .size = size,
        .offset = offset,
        .scope = scope };

    // Just search the current scope.
    for (int i = 0; i < SYMBOL_TABLE_MAX_SIZE; i++) {
        // Match characterisitcs from easiest to hardest
        if (!symbol_table_live[i]) {
            continue;
        }
        symbol_table_entry_t search_entry = symbol_table[i];
        if (new_entry.scope != search_entry.scope)
            continue;

        if (!strcmp(new_entry.identifier, search_entry.identifier)) {
            // Error, variable already decalred.
            printf("Error: symbol %s already declared\n", new_entry.identifier);
            return;
        }
    }
    symbol_table_live[idx] = true;
    symbol_table[idx++] = new_entry;
    return;

}