#include <string.h>
#include <stdio.h>

#include "symbol_table.h"
#include "parser.h"
#include "token.h"
#include "error.h"

// Parent pointers:
symbol_table_entry_t symbol_table_search(symbol_table_t* symbol_table, token_t identifier_token, i32 scope) {
    char* identifier = identifier_token.contents;
    for (int i = 0; i < symbol_table->idx; i++) {
        // Match characterisitcs from easiest to hardest
        symbol_table_entry_t entry = symbol_table->data[i];
        if (scope != entry.scope)
            continue;

        if (!strcmp(identifier, entry.identifier)) {
            return entry;
        }
    }
    // Recurse up, return the first match.
    if (scope != 0) {
        // Parent pointer.
        return symbol_table_search(symbol_table, identifier_token, symbol_table->parent_scope[scope]);
    } 
    symbol_table_entry_t invalid = {0};

    parser_error_t error = {
        .invalid_token = identifier_token,
        .prev_token = identifier_token,
        .type = ERROR_SYMBOL_UNKNOWN
    };
    report_error(error);
    return invalid;
}

void symbol_table_add(symbol_table_t* symbol_table, token_t identifier_token, i32 scope, type_info_t return_type, i32 type, i32 size, i32 offset) {
    char* identifier = identifier_token.contents;
    
    symbol_table_entry_t new_entry = {
        .identifier = identifier, 
        .type_info = return_type,
        .type = type,
        .size = size,
        .offset = offset,
        .scope = scope 
    };

    // Just search the current scope.
    for (int i = 0; i < symbol_table->idx; i++) {
        // Match characterisitcs from easiest to hardest
        symbol_table_entry_t search_entry = symbol_table->data[i];
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
    //symbol_table_live[idx] = true;
    symbol_table->data[symbol_table->idx++] = new_entry;
    return;
}
