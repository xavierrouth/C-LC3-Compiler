#include "type_table.h"

// TODO: Support pointing pointers, and more than one type.
// Split static and voltaile etc into things that are not type.
char* type_info_to_str(type_info_t type_info) {
    switch (type_info.type) {
        case INT: return "INT";
        case CHAR: return "CHAR";
        case PTR: return "PTR";
        case VOID: return "VOID";
        default: return "Uknonwn";   
    }
}