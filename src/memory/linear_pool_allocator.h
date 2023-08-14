#if 0
#include "uniform_allocator.h"

uniform_allocator_t init_uniform_allocator(uint8_t* start, uint8_t* end, size_t ele_size) {
    assert(start <= end);
    uniform_allocator_t allocator = {
        .start = start,
        .end = end,
        .ptr = end
    };
    return allocator;
}

void* uniform_allocate(uniform_allocator_t* allocator, size_t align, size_t size) {
    // Downwards uniforming.
    
    // TOOD: assert that align is a power of two
    
    // Decrement pointer, and then round down by masking off low bits
    uint8_t* new_ptr = (uintptr_t)(allocator->ptr - size) & (uintptr_t) ~(align - 1);

    if (allocator->start <= new_ptr)
        return NULL;

    allocator->ptr = new_ptr;
    return (void*)allocator->ptr;
}

void uniform_free(uniform_allocator_t* allocator) {
    allocator->ptr = allocator->end;
}
#endif