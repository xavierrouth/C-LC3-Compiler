// std::vector

#ifndef T
#error "Template type T undefined for vector."
#endif

#include "util.h"
#include <stdlib.h>

#ifndef NAME
#define NAME T
#endif

#define V JOIN(NAME, vector)



// For highlighting:

//#ifdef T

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
    container.data = malloc(sizeof(T) * container.capacity);
    return container;
}

static void JOIN(V, push)(V* container, T element) {
    if (container->size == container->capacity) {
        container->capacity *= 2;
        T* tmp = NULL;
        tmp = realloc(container->data, container->capacity * sizeof(T));
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
    free(container.data);
    return;
}

#undef V
