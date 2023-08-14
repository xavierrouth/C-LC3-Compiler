#include "bump_allocator.h"

bump_allocator_t init_bump_allocator(uint8_t* start, uint8_t* end) {
    assert(start <= end);
    bump_allocator_t allocator = {
        .start = start,
        .end = end,
        .ptr = end
    };
    return allocator;
}

void* bump_allocate(bump_allocator_t* allocator, size_t align, size_t size) {
    // Downwards bumping.
    
    // TOOD: assert that align is a power of two
    
    // Decrement pointer, and then round down by masking off low bits
    uint8_t* new_ptr = (uintptr_t)(allocator->ptr - size) & (uintptr_t) ~(align - 1);

    if (allocator->start > new_ptr) {
        printf("out of mem\n");
        return NULL;
    }
        

    allocator->ptr = new_ptr;
    return (void*)allocator->ptr;
}

void bump_free(bump_allocator_t* allocator) {
    allocator->ptr = allocator->end;
}