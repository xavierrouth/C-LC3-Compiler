#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

#include "token.h"

#define MAX_DECL_PARTS 8

typedef struct SPECIFIER_INFO_STRUCT {
    // Type Qualifiers
    unsigned int is_const : 1;
    unsigned int is_voltatile : 1;
    // Storage Specifiers
    unsigned int is_static : 1;
    /*unsigned int is_auto : 1;
    unsigned int is_extern : 1;
    unsigned int is_register 1:  */
    // Other Thing: 
    unsigned int is_int : 1;
    unsigned int is_char : 1;
    unsigned int is_void : 1;
    // TODO:
    // Tags
    // uint16_t struct_tag; 
    // uint16_t enum_tag;
    // Typedef
    // uint16_t typedef_id;
} specifier_info_t;

typedef struct DECLARATOR_PART {
    enum {
        UNDEFINED_DECL,
        FUNCTION_DECL,
        POINTER_DECL,
        ARRAY_DECL,
    } type;
    union {
        specifier_info_t pointer_subtype; // 'const' or etc. Can be bare.
        specifier_info_t function_subtype; // Function Parameters
        uint16_t array_size; // Size of array
    };
} declarator_part_t;

typedef struct DECLARATOR_TYPE {
    declarator_part_t parts[MAX_DECL_PARTS];
    uint16_t idx;
} declarator_t;

typedef struct TYPE_INFO_STRUCT {
    specifier_info_t specifier_info;
    declarator_t declarator;
    token_t identifier_token;
} type_info_t;

char* type_info_to_str(type_info_t type_info);

/** Append b to the end of a*/
declarator_t merge_declarator(declarator_t a, declarator_t b);

#endif
