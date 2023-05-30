#ifndef HEAP_H
#define HEAP_H

#include <stdio.h>
#include <unistd.h>

#include "chunk.h"

typedef struct {
  FILE* file;
} Heap;

Heap* heap_init(Heap* h, const char* name);
void heap_destroy(Heap* heap);

unsigned long heap_insert(Heap* heap, Chunk* chunk);
void heap_write(Heap* heap, unsigned long address, Chunk* chunk);
Chunk* heap_get(Heap* heap, unsigned long address);

#endif // HEAP_H
