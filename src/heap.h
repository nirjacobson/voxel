#ifndef HEAP_H
#define HEAP_H

#include <stdio.h>
#include <unistd.h>

#include "chunk.h"

typedef struct {
  unsigned long freeSpacePtr;
} HeapHeader;

typedef struct {
  int width;
  int height;
  int length;
} HeapEntry;

typedef struct {
  FILE* file;
} Heap;

Heap* heap_init(Heap* h, const char* name);
void heap_destroy(Heap* heap);

void heap_init_heap(Heap* heap);

HeapHeader heap_get_header(Heap* heap);
void heap_set_header(Heap* heap, HeapHeader* header);

unsigned long heap_insert(Heap* heap, Chunk* chunk);
void heap_write(Heap* heap, unsigned long address, Chunk* chunk);
Chunk* heap_get(Heap* heap, unsigned long address);

#endif // HEAP_H
