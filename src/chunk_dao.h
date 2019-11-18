#ifndef CHUNK_DAO_H
#define CHUNK_DAO_H

#include "b_tree.h"
#include "heap.h"

typedef struct {
  BTree btree;
  Heap heap;
} ChunkDAO;

ChunkDAO* chunk_dao_init(ChunkDAO* cd, const char* worldName);
void chunk_dao_destroy(ChunkDAO* chunkDAO);

void chunk_dao_save(ChunkDAO* chunkDAO, ChunkID* chunkID, Chunk* chunk);
Chunk* chunk_dao_load(ChunkDAO* chunkDAO, ChunkID* chunkID);

#endif // CHUNK_DAO_H
