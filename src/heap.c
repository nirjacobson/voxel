#include "heap.h"

Heap* heap_init(Heap* h, const char* name) {
  Heap* heap = h ? h : NEW(Heap, 1);

  char filename[16];
  sprintf(filename, "%s.vxl", name);
  if (access(filename, F_OK) == -1) {
    heap->file = fopen(filename, "w+b");
  } else {
    heap->file = fopen(filename, "r+b");
  }

  fseek(heap->file, 0, SEEK_END);
  if (ftell(heap->file) < sizeof(HeapHeader)) {
    heap_init_heap(heap);
  }

  return heap;
}

void heap_destroy(Heap* heap) {
  fclose(heap->file);
}


HeapHeader heap_get_header(Heap* heap) {
  HeapHeader header;
  fseek(heap->file, 0, SEEK_SET);
  fread(&header, sizeof(HeapHeader), 1, heap->file);

  return header;
}
void heap_set_header(Heap* heap, HeapHeader* header) {
  fseek(heap->file, 0, SEEK_SET);
  fwrite(header, sizeof(HeapHeader), 1, heap->file);
}

void heap_init_heap(Heap* heap) {
  HeapHeader header = { sizeof(HeapHeader) };
  heap_set_header(heap, &header);
}

void heap_write(Heap* heap, unsigned long address, Chunk* chunk) {
  HeapEntry entry;
  entry.width = chunk->width;
  entry.height = chunk->height;
  entry.length = chunk->length;
  int numBlocks = entry.width * entry.height * entry.length;

  Block* blocks = NEW(Block, numBlocks);
  for (int x = 0; x < entry.width; x++) {
    for (int y = 0; y < entry.height; y++) {
      for (int z = 0; z < entry.length; z++) {
        int index = x * (entry.height * entry.length) + y * entry.length + z;
        blocks[index] = chunk->blocks[x][y][z];
      }
    }
  }
  fseek(heap->file, address, SEEK_SET);
  fwrite(&entry, sizeof(HeapEntry), 1, heap->file);
  fwrite(blocks, numBlocks*sizeof(Block), 1, heap->file);
  free(blocks);
}

unsigned long heap_insert(Heap* heap, Chunk* chunk) {
  HeapHeader header = heap_get_header(heap);

  unsigned long address = header.freeSpacePtr;

  heap_write(heap, address, chunk);

  header.freeSpacePtr += sizeof(HeapEntry) + chunk->width * chunk->height * chunk->length * sizeof(Block);
  heap_set_header(heap, &header);

  return address;
}

Chunk* heap_get(Heap* heap, unsigned long address) {
  HeapEntry entry;
  fseek(heap->file, address, SEEK_SET);
  fread(&entry, sizeof(HeapEntry), 1, heap->file);
  int numBlocks = entry.width * entry.height * entry.length;

  Block* blocks = NEW(Block, numBlocks);
  fread(blocks, numBlocks*sizeof(Block), 1, heap->file);

  Chunk* chunk = chunk_init(NULL, entry.width, entry.height, entry.length);
  chunk->width = entry.width;
  chunk->height = entry.height;
  chunk->length = entry.length;

  for (int i = 0; i < entry.width * entry.height * entry.length; i++) {
    unsigned int x = i / (entry.height * entry.length);
    unsigned int y = (i % (entry.height * entry.length)) / entry.length;
    unsigned int z = (i % (entry.height * entry.length)) % entry.length;
    chunk->blocks[x][y][z] = blocks[i];
  }
  
  free(blocks);

  return chunk;
}
