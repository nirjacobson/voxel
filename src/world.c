#include "world.h"
#include "internal/world.h"

/* Linked list processing callbacks */

void destroy_world_chunk(void* worldChunkPtr) {
    WorldChunk* worldChunk = (WorldChunk*)worldChunkPtr;

    chunk_destroy(worldChunk->chunk);
    free(worldChunk->chunk);
    free(worldChunk);
}

/* World */

World* world_init(World* world, const char* name) {
    World* w = world ? world : NEW(World, 1);

    linked_list_init(&w->chunks);

    ground_init(&w->ground, 500);

    return w;
}

void world_destroy(World* world) {
    ground_destroy(&world->ground);
    linked_list_destroy(&world->chunks, destroy_world_chunk);
}

