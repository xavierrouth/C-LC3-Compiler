#include <string.h>
#include <stdio.h>

#include "symbol_table.h"
#include "parser.h"
#include "token.h"
#include "error.h"

symbol_table_entry_t symbol_table[SYMBOL_TABLE_MAX_SIZE];
bool symbol_table_live[SYMBOL_TABLE_MAX_SIZE];

// Parent pointers:
int32_t parent_scope[SYMBOL_TABLE_MAX_SIZE];

symbol_table_entry_t symbol_table_search(token_t identifier_token, int32_t scope) {
    char* identifier = identifier_token.contents;
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
        return symbol_table_search(identifier_token, parent_scope[scope]);
    } 
    symbol_table_entry_t invalid = {0};
    // TODO: Error:
    parser_error_t error = {
        .invalid_token = identifier_token,
        .prev_token = identifier_token,
        .type = ERROR_SYMBOL_UNKNOWN
    };
    report_error(error);
    return invalid;
}

void symbol_table_add(token_t identifier_token, int32_t scope, type_info_t return_type, int32_t type, int32_t size, int32_t offset) {
    static int32_t idx = 0;
    char* identifier = identifier_token.contents;
    
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
            // TODO: Point to previous definition
            parser_error_t error = {
                .invalid_token = identifier_token,
                .prev_token = identifier_token,
                .type = ERROR_SYMBOL_REDECLARED
            };
            report_error(error);
            return;
        }
    }
    symbol_table_live[idx] = true;
    symbol_table[idx++] = new_entry;
    return;
}
