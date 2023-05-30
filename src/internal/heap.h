#ifndef HEAP_INTERNAL_H
#define HEAP_INTERNAL_H

#include "../heap.h"

typedef struct {
  unsigned long freeSpacePtr;
} HeapHeader;

typedef struct {
  int width;
  int height;
  int length;
} HeapEntry;

void heap_init_heap(Heap* heap);

HeapHeader heap_get_header(Heap* heap);
void heap_set_header(Heap* heap, HeapHeader* header);

#endif // HEAP_INTERNAL_H