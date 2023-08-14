#if 0
#include "linear_pool_allocator.h"

linear_pool_allocator_t init_linear_pool_allocator(uint8_t* start, uint8_t* end, size_t ele_size) {
    assert(start <= end);
    linear_pool_allocator_t allocator = {
        .start = start,
        .end = end,
        .ptr = end
    };
    return allocator;
}

void* linear_pool_allocate(linear_pool_allocator_t* allocator, size_t align, size_t size) {
    // Downwards linear_pooling.
    
    // TOOD: assert that align is a power of two
    
    // Decrement pointer, and then round down by masking off low bits
    uint8_t* new_ptr = (uintptr_t)(allocator->ptr - size) & (uintptr_t) ~(align - 1);

    if (allocator->start <= new_ptr)
        return NULL;

    allocator->ptr = new_ptr;
    return (void*)allocator->ptr;
}

void linear_pool_free(linear_pool_allocator_t* allocator) {
    allocator->ptr = allocator->end;
}
#endif