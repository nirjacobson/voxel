#include "chunk_dao.h"

ChunkDAO* chunk_dao_init(ChunkDAO* cd, const char* worldName) {
    ChunkDAO* chunkDAO = cd ? cd : NEW(ChunkDAO, 1);

    heap_init(&chunkDAO->heap, worldName);
    bp_tree_init(&chunkDAO->bptree, worldName);

    return chunkDAO;
}

void chunk_dao_destroy(ChunkDAO* chunkDAO) {
    bp_tree_destroy(&chunkDAO->bptree);
    heap_destroy(&chunkDAO->heap);
}

void chunk_dao_save(ChunkDAO* chunkDAO, ChunkID* chunkID, Chunk* chunk) {
    unsigned long address;
    if (bp_tree_find(&chunkDAO->bptree, chunkID, &address)) {
        heap_write(&chunkDAO->heap, address, chunk);
    } else {
        bp_tree_insert(&chunkDAO->bptree, chunkID, heap_insert(&chunkDAO->heap, chunk));
    }
}

Chunk* chunk_dao_load(ChunkDAO* chunkDAO, ChunkID* chunkID) {
    unsigned long address;
    if (bp_tree_find(&chunkDAO->bptree, chunkID, &address)) {
        return heap_get(&chunkDAO->heap, address);
    }

    return NULL;
}
