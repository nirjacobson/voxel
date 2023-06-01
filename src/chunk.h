#ifndef CHUNK_H
#define CHUNK_H

#include "block.h"
#include "mesh.h"
#include "linked_list.h"

typedef struct {
  int x;
  int y;
  int z;
} ChunkID;

typedef struct {
  Block*** blocks;
  LinkedList meshes;
  int width;
  int height;
  int length;
  char dirty;
} Chunk;

/* Chunk */

Chunk* chunk_init(Chunk* c, int width, int height, int length);
void chunk_destroy(Chunk* chunk);

void chunk_mesh(Chunk* chunk);

#endif // CHUNK_H
