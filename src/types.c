// TODO: Support pointing pointers, and more than one type.
// Split static and voltaile etc into things that are not type.
#include "types.h"

char buffer[128];

char* type_info_to_str(type_info_t type_info) {
    strcpy(buffer, ""); // Clear the buffer??
    if (type_info.specifier_info.is_int) {
        strcat(buffer, "int ");
    }
    if (type_info.declarator_part_list[0].type == POINTER_DECL) {
        strcat(buffer, "*");
    }
    if (buffer[0] == '\0') {
        return "UNKONWN_TYPE";
    }
    return &buffer[0];
}