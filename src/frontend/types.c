// TODO: Support pointing pointers, and more than one type.
// Split static and voltaile etc into things that are not type.
#include <string.h>
#include <stdbool.h>

#include "types.h"
#include "util/util.h"

static char decl_buffer[128];
static u8 buffer_idx = 0;

char* print_declarator(declarator_t a, bool prev_fnc_or_array) {
    
    if (a.idx == 0) {
        return &decl_buffer[0];
    }
    else {
        declarator_part_t part = a.parts[a.idx - 1];
        declarator_t prev = a;
        prev.idx--;
        if (part.type == FUNCTION_DECL || part.type == ARRAY_DECL) {
            
            
            print_declarator(prev, true);
            if (part.type == ARRAY_DECL) {
                buffer_idx += sprintf(decl_buffer + buffer_idx, "[%d]", part.array_size);
            }
            else {
                printf("memer\n");
            }
            
        }
        // Poitner declaration part
        else {
            if (prev_fnc_or_array) {
                strcat(decl_buffer, "(");
                buffer_idx++;
            }
            strcat(decl_buffer, "*");
            buffer_idx ++;
            print_declarator(prev, false);

            if (prev_fnc_or_array) {
                strcat(decl_buffer, ")");
                buffer_idx++;
            }

        }
    }
    return &decl_buffer[0];
}

static char buffer[128];

char* type_info_to_str(type_info_t type_info) {
    strcpy(buffer, ""); // Clear the buffer??
    if (type_info.specifier_info.is_int) {
        strcat(buffer, "int ");
    }
    
    strcat(buffer, print_declarator(type_info.declarator, false));
    strcpy(decl_buffer, "");
    buffer_idx = 0;
    if (buffer[0] == '\0') {
        return "UNKONWN_TYPE";
    }
    
    return &buffer[0];
}

declarator_t merge_declarator(declarator_t a, declarator_t b) {
    for (int i = 0; i < b.idx; i++) {
        a.parts[i + a.idx] = b.parts[i];
    }
    a.idx += b.idx;
    return a;
}
