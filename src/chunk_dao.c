#include "chunk_dao.h"

ChunkDAO* chunk_dao_init(ChunkDAO* cd, const char* worldName) {
  ChunkDAO* chunkDAO = cd ? cd : NEW(ChunkDAO, 1);

  heap_init(&chunkDAO->heap, worldName);
  b_tree_init(&chunkDAO->btree, worldName);

  return chunkDAO;
}

void chunk_dao_destroy(ChunkDAO* chunkDAO) {
  b_tree_destroy(&chunkDAO->btree);
  heap_destroy(&chunkDAO->heap);
}

void chunk_dao_save(ChunkDAO* chunkDAO, ChunkID* chunkID, Chunk* chunk) {
  unsigned int address;
  if (b_tree_find(&chunkDAO->btree, chunkID, &address)) {
    heap_write(&chunkDAO->heap, address, chunk);
  } else {
    b_tree_insert(&chunkDAO->btree, chunkID,
      heap_insert(&chunkDAO->heap, chunk));
  }
}

Chunk* chunk_dao_load(ChunkDAO* chunkDAO, ChunkID* chunkID) {
  unsigned int address;
  if (b_tree_find(&chunkDAO->btree, chunkID, &address)) {
    return heap_get(&chunkDAO->heap, address);
  }

  return NULL;
}
