#ifndef WORLD_H
#define WORLD_H

#define WORLD_CHUNK_LENGTH    16

#include <stdlib.h>

#include "box.h"
#include "camera.h"
#include "chunk.h"
#include "chunk_dao.h"
#include "linked_list.h"
#include "ground.h"

typedef struct {
    ChunkID id;
    Chunk* chunk;
} WorldChunk;

typedef struct {
    Vulkan* vulkan;
    ChunkDAO chunkDAO;
    LinkedList chunks;
    Ground ground;
} World;

World* world_init(World* world, Vulkan* vulkan, const char* name);
void world_destroy(World* world);

Block* world_get_block(World* world, int* location);
void world_block_set_active(World* world, int* location, char active);
void world_block_set_color(World* world, int* location, uint16_t color);

Chunk* world_copy_chunk(World* world, Box* box);
Chunk* world_cut_chunk(World* world, Box* box);
void world_set_chunk(World* world, Chunk* chunk, int* location, int rotation);
void world_clear_region(World* world, Box* region);

void world_update(World* world, Camera* camera);

#endif // WORLD_H
