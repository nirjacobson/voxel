#ifndef WORLD_H
#define WORLD_H

#define WORLD_CHUNK_LENGTH    16

#include <stdlib.h>

#include "camera.h"
#include "chunk.h"
#include "linked_list.h"
#include "ground.h"

typedef struct {
    ChunkID id;
    Chunk* chunk;
} WorldChunk;

typedef struct {
    LinkedList chunks;
    Ground ground;
} World;

World* world_init(World* world, const char* name);
void world_destroy(World* world);

Block* world_get_block(World* world, int* location);
void world_block_set_active(World* world, int* location, char active);
void world_block_set_color(World* world, int* location, uint16_t color);

#endif // WORLD_H
