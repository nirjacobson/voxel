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

#endif // WORLD_H
