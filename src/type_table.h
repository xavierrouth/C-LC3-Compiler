#ifndef TYPE_TABLE_H
#define TYPE_TABLE_H

#include <stdbool.h>

typedef enum TYPE_ENUM {
    INT,
    CHAR,
    PTR,
    VOID,
    NOTYPE,
} type_enum;

typedef struct VAR_INFO_STRUCT {
    type_enum type;
    bool is_pointer;
} type_info_t;

#endif