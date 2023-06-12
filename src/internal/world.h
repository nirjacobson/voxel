#ifndef WORLD_INTERNAL_H
#define WORLD_INTERNAL_H

#include "../world.h"

/* Linked list processing callbacks */

void load_world_chunk(void* chunkIDPtr, void* worldPtr);
void unload_world_chunk(void* worldChunkPtr, void* worldPtr);
char chunk_id_equals_world_chunk(void* chunkIDPtr, void* worldChunkPtr);
void destroy_world_chunk(void* worldChunkPtr);
int compare_chunk_ids(ChunkID* chunkIDA, ChunkID* chunkIDB);
int compare_world_chunks(void* worldChunkAPtr, void* worldChunkBPtr);

/* World */

Chunk* world_load_world_chunk(World* world, ChunkID* chunkID);
void world_unload_world_chunk(World* world, WorldChunk* worldChunk);

LinkedList* world_draw_list(Camera* camera);

#endif // WORLD_INTERNAL_H
