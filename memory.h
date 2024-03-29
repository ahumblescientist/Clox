#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdio.h>

#define ALLOCATE(type, size) (type *)reallocate(NULL, 0, sizeof(type) * size)
#define GROW_SIZE(oldSize) (!oldSize ? 8 : oldSize*2)
#define GROW_ARRAY(type, pointer, oldSize, size) (type *)reallocate(pointer, sizeof(type) * oldSize, sizeof(type) * size)
#define FREE_ARRAY(type, pointer, size) reallocate(pointer, sizeof(type) * size, 0)
#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

void *reallocate(void *, size_t, size_t);
void freeObjects();

#endif
