#ifndef WORLD_H
#define WORLD_H

#define WORLD_CHUNK_LENGTH    16

#include <stdlib.h>

#include "camera.h"
#include "chunk.h"
#include "chunk_dao.h"
#include "linked_list.h"
#include "box.h"
#include "ground.h"

typedef struct {
    ChunkID id;
    Chunk* chunk;
} WorldChunk;

typedef struct {
    ChunkDAO chunkDAO;
    LinkedList chunks;
    Ground ground;
} World;

World* world_init(World* world, const char* name);
void world_destroy(World* world);

Chunk* world_load_world_chunk(World* world, ChunkID* chunkID);
void world_unload_world_chunk(World* world, WorldChunk* worldChunk);

Block* world_get_block(World* world, int* location);
void world_block_set_active(World* world, int* location, char active);
void world_block_set_color(World* world, int* location, uint16_t color);

LinkedList* world_draw_list(Camera* camera);
void world_draw(World* world, Camera* camera, Renderer* renderer);

Chunk* world_copy_chunk(World* world, Box* box);
Chunk* world_cut_chunk(World* world, Box* box);
void world_set_chunk(World* world, Chunk* chunk, int* location, int rotation);

#endif // WORLD_H
