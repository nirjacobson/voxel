#ifndef WORLD_INTERNAL_H
#define WORLD_INTERNAL_H

#include "../world.h"

/* Linked list processing callbacks */
char chunk_id_equals_world_chunk(void* chunkIDPtr, void* worldChunkPtr);
void destroy_world_chunk(void* worldChunkPtr);
int compare_chunk_ids(ChunkID* chunkIDA, ChunkID* chunkIDB);
int compare_world_chunks(void* worldChunkAPtr, void* worldChunkBPtr);

#endif // WORLD_INTERNAL_H
