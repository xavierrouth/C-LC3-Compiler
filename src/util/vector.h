// std::vector

#ifndef T
#error "Template type T undefined for vector."
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util.h"
#include "memory/bump_allocator.h"

#ifndef NAME
#define NAME T
#endif

#define V JOIN(NAME, vector)

#define VECTOR_ALLOCATOR_SIZE 2048 * 16

static char buffer[VECTOR_ALLOCATOR_SIZE];

static bump_allocator_t vector_allocator = {
    .start = &buffer[0],
    .end = &buffer[VECTOR_ALLOCATOR_SIZE - 1],
    .ptr = &buffer[VECTOR_ALLOCATOR_SIZE - 1]
};

typedef struct V {
    T* data;
    int size;
    int capacity;
} V;

// Initial number of items expected to be stored in this container
static V JOIN(V, init)(int initial_capacity) {
    V container;
    container.size = 0;
    container.capacity = initial_capacity;
    container.data = (T*) bump_allocate(&vector_allocator, sizeof(T), sizeof(T) * container.capacity);
    return container;
}

static void JOIN(V, push)(V* container, T element) {
    if (container->size == container->capacity) {
        
        T* tmp = NULL;
        // reallocate tmp = realloc(container->data, container->capacity * sizeof(T));
        tmp = (T*) bump_allocate(&vector_allocator, sizeof(T), sizeof(T) * container->capacity * 2);
        for (int i = 0; i < container->capacity; i++) {
            tmp[i] = container->data[i];
        }
        container->capacity *= 2;
        
        if (tmp == NULL) {
            // Fail::
            printf("Realloc Failed\n");
            return;
        }
        else {
            container->data = tmp;
        }
    }
    container->data[container->size++] = element;
}

static T JOIN(V, access)(V* container, int index) {
    if (index > container->size) {
        printf("Vector index access out of bounds.\n");
        T ret = {0};
        return ret;
    }
    return container->data[index];
}

static void JOIN(V, replace)(V* container, T element, int index) {
    if (index > container->size) {
        printf("Vector index access out of bounds.\n");
        return;
    }
    container->data[index] = element;
    return;
}

// This frees the list, this does not free the objects in the lsit.
static void JOIN(V, free)(V container) {
    return;
}

#undef V
