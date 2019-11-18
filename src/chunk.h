#ifndef CHUNK_H
#define CHUNK_H

#include "block.h"
#include "mesh.h"
#include "linked_list.h"

typedef struct {
    uint16_t color;
    Mesh mesh;
} MeshListNode;

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

/* Linked list processing callbacks */

void mesh_list_node_mesh_destroy(void* node);
char mesh_list_nodes_equal(void* nodeA, void* nodeB) ;
void mesh_list_node_prepare(void* node, void* rendererPtr);
void mesh_list_node_draw(void* node, void* renderer);

/* Chunk */

Chunk* chunk_init(Chunk* c, int width, int height, int length);
void chunk_destroy(Chunk* chunk);

void chunk_mesh(Chunk* chunk);
void chunk_draw(Chunk* chunk, Renderer* renderer, float* position);

void chunk_block_set_active(Chunk* chunk, int x, int y, int z, char active);
void chunk_block_set_color(Chunk* chunk, int x, int y, int z, uint16_t color);

#endif // CHUNK_H
