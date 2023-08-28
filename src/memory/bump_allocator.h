/**
 * Simple bump allocator.
*/

#ifndef BUMP_ALLOCA_H
#define BUMP_ALLOCA_H

#include <stdint.h>
#include <assert.h>
#include <stddef.h>

#include "util/util.h"

// Include a callback for out of memory errors

typedef struct {
    // start <= ptr <= end
    u8* start;
    u8* end;
    u8* ptr;
} bump_allocator_t;

bump_allocator_t init_bump_allocator(u8* start, u8* end);

void* bump_allocate(bump_allocator_t* allocator, size_t align, size_t size);

void bump_free(bump_allocator_t* allocator);

#endif
