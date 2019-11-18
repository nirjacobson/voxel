#ifndef HEAP_H
#define HEAP_H

#include <stdio.h>
#include <unistd.h>

#include "chunk.h"

typedef struct {
  unsigned int freeSpacePtr;
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

unsigned int heap_insert(Heap* heap, Chunk* chunk);
void heap_write(Heap* heap, unsigned int address, Chunk* chunk);
Chunk* heap_get(Heap* heap, unsigned int address);

#endif // HEAP_H
