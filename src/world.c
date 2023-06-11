#include "world.h"
#include "internal/world.h"

/* Linked list processing callbacks */

void destroy_world_chunk(void* worldChunkPtr) {
    WorldChunk* worldChunk = (WorldChunk*)worldChunkPtr;

    chunk_destroy(worldChunk->chunk);
    free(worldChunk->chunk);
    free(worldChunk);
}

char chunk_id_equals_world_chunk(void* chunkIDPtr, void* worldChunkPtr) {
    ChunkID* chunkID = (ChunkID*)chunkIDPtr;
    WorldChunk* worldChunk = (WorldChunk*)worldChunkPtr;

    return chunkID->x == worldChunk->id.x &&
           chunkID->y == worldChunk->id.y &&
           chunkID->z == worldChunk->id.z;
}

int compare_chunk_ids(ChunkID* chunkIDA, ChunkID* chunkIDB) {
    if (chunkIDA->x == chunkIDB->x) {
        if (chunkIDA->y == chunkIDB->y) {
            return chunkIDA->z - chunkIDB->z;
        }
        return chunkIDA->y - chunkIDB->y;
    }
    return chunkIDA->x - chunkIDB->x;
}

int compare_world_chunks(void* worldChunkAPtr, void* worldChunkBPtr) {
    WorldChunk* worldChunkA = (WorldChunk*)worldChunkAPtr;
    WorldChunk* worldChunkB = (WorldChunk*)worldChunkBPtr;

    return compare_chunk_ids(&worldChunkA->id, &worldChunkB->id);
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

Block* world_get_block(World* world, int* location) {
    int chunk_position[] = {
        floor((float)location[0] / WORLD_CHUNK_LENGTH),
        floor((float)location[1] / WORLD_CHUNK_LENGTH),
        floor((float)location[2] / WORLD_CHUNK_LENGTH)
    };
    int block_position[] = {
        location[0] % WORLD_CHUNK_LENGTH,
        location[1] % WORLD_CHUNK_LENGTH,
        location[2] % WORLD_CHUNK_LENGTH
    };

    if (block_position[0] < 0)
        block_position[0] += WORLD_CHUNK_LENGTH;
    if (block_position[1] < 0)
        block_position[1] += WORLD_CHUNK_LENGTH;
    if (block_position[2] < 0)
        block_position[2] += WORLD_CHUNK_LENGTH;

    ChunkID chunkID;
    chunkID.x = chunk_position[0];
    chunkID.y = chunk_position[1];
    chunkID.z = chunk_position[2];

    LinkedListNode* chunkNode = linked_list_find(&world->chunks, &chunkID, chunk_id_equals_world_chunk);

    if (chunkNode) {
        WorldChunk* worldChunk = (WorldChunk*)chunkNode->data;
        Block* block = &worldChunk->chunk->blocks[block_position[0]][block_position[1]][block_position[2]];

        return block;
    } else {
        return NULL;
    }
}

void world_block_set_active(World* world, int* location, char active) {
    int chunk_position[] = {
        floor((float)location[0] / WORLD_CHUNK_LENGTH),
        floor((float)location[1] / WORLD_CHUNK_LENGTH),
        floor((float)location[2] / WORLD_CHUNK_LENGTH)
    };
    int block_position[] = {
        location[0] % WORLD_CHUNK_LENGTH,
        location[1] % WORLD_CHUNK_LENGTH,
        location[2] % WORLD_CHUNK_LENGTH
    };

    if (block_position[0] < 0)
        block_position[0] += WORLD_CHUNK_LENGTH;
    if (block_position[1] < 0)
        block_position[1] += WORLD_CHUNK_LENGTH;
    if (block_position[2] < 0)
        block_position[2] += WORLD_CHUNK_LENGTH;

    ChunkID chunkID;
    chunkID.x = chunk_position[0];
    chunkID.y = chunk_position[1];
    chunkID.z = chunk_position[2];

    LinkedListNode* chunkNode = linked_list_find(&world->chunks, &chunkID, chunk_id_equals_world_chunk);

    Chunk* chunk;
    if (chunkNode) {
        WorldChunk* worldChunk = (WorldChunk*)chunkNode->data;
        chunk = worldChunk->chunk;
    } else {
        chunk = chunk_init(NULL, WORLD_CHUNK_LENGTH, WORLD_CHUNK_LENGTH, WORLD_CHUNK_LENGTH);
        WorldChunk* worldChunk = NEW(WorldChunk, 1);
        worldChunk->id = chunkID;
        worldChunk->chunk = chunk;
        linked_list_insert_ordered(&world->chunks, worldChunk, compare_world_chunks);
    }

    Block* block = &chunk->blocks[block_position[0]][block_position[1]][block_position[2]];
    block_set_active(block, active);
    chunk_mesh(chunk);
}

void world_block_set_color(World* world, int* location, uint16_t color) {
    int chunk_position[] = {
        floor((float)location[0] / WORLD_CHUNK_LENGTH),
        floor((float)location[1] / WORLD_CHUNK_LENGTH),
        floor((float)location[2] / WORLD_CHUNK_LENGTH)
    };
    int block_position[] = {
        location[0] % WORLD_CHUNK_LENGTH,
        location[1] % WORLD_CHUNK_LENGTH,
        location[2] % WORLD_CHUNK_LENGTH
    };

    if (block_position[0] < 0)
        block_position[0] += WORLD_CHUNK_LENGTH;
    if (block_position[1] < 0)
        block_position[1] += WORLD_CHUNK_LENGTH;
    if (block_position[2] < 0)
        block_position[2] += WORLD_CHUNK_LENGTH;

    ChunkID chunkID;
    chunkID.x = chunk_position[0];
    chunkID.y = chunk_position[1];
    chunkID.z = chunk_position[2];

    LinkedListNode* chunkNode = linked_list_find(&world->chunks, &chunkID, chunk_id_equals_world_chunk);

    Chunk* chunk;
    if (chunkNode) {
        WorldChunk* worldChunk = (WorldChunk*)chunkNode->data;
        chunk = worldChunk->chunk;
    } else {
        chunk = chunk_init(NULL, WORLD_CHUNK_LENGTH, WORLD_CHUNK_LENGTH, WORLD_CHUNK_LENGTH);
        WorldChunk* worldChunk = NEW(WorldChunk, 1);
        worldChunk->id = chunkID;
        worldChunk->chunk = chunk;
        linked_list_insert_ordered(&world->chunks, worldChunk, compare_world_chunks);
    }

    Block* block = &chunk->blocks[block_position[0]][block_position[1]][block_position[2]];
    block_set_color(block, color);
    chunk_mesh(chunk);
}

Chunk* world_copy_chunk(World* world, Box* box) {
    Chunk* chunk = chunk_init(NULL, box->width, box->height, box->length);
    for (int x = 0; x < box->width; x++) {
        for (int y = 0; y < box->height; y++) {
            for (int z = 0; z < box->length; z++) {
                int location[3] = {
                    box->position[0] + x,
                    box->position[1] + y,
                    box->position[2] + z
                };

                Block* block = world_get_block(world, location);

                if (block) {
                    chunk->blocks[x][y][z].data = block->data;
                }
            }
        }
    }

    return chunk;
}

Chunk* world_cut_chunk(World* world, Box* box) {
    Chunk* chunk = chunk_init(NULL, box->width, box->height, box->length);
    for (int x = 0; x < box->width; x++) {
        for (int y = 0; y < box->height; y++) {
            for (int z = 0; z < box->length; z++) {
                int location[3] = {
                    box->position[0] + x,
                    box->position[1] + y,
                    box->position[2] + z
                };

                Block* block = world_get_block(world, location);

                if (block) {
                    chunk->blocks[x][y][z].data = block->data;
                }

                world_block_set_color(world, location, 0);
                world_block_set_active(world, location, 0);
            }
        }
    }

    return chunk;
}

void world_set_chunk(World* world, Chunk* chunk, int* location, int rotation) {
    for (int x = 0; x < chunk->width; x++) {
        for (int y = 0; y < chunk->height; y++) {
            for (int z = 0; z < chunk->length; z++) {
                Block* block = &chunk->blocks[x][y][z];
                int blockLocation[3];

                blockLocation[1] = location[1] + y;
                switch (rotation) {
                    case 0:
                        blockLocation[0] = location[0] + x;
                        blockLocation[2] = location[2] + z;
                        break;
                    case 1:
                        blockLocation[0] = location[0] + z;
                        blockLocation[2] = location[2] - x - 1 + chunk->width;
                        break;
                    case 2:
                        blockLocation[0] = location[0] - x - 1 + chunk->width;
                        blockLocation[2] = location[2] - z - 1 + chunk->length;
                        break;
                    case 3:
                        blockLocation[0] = location[0] - z - 1 + chunk->length;
                        blockLocation[2] = location[2] + x;
                        break;
                }
                if (block_is_active(block)) {
                    world_block_set_active(world, blockLocation, 1);
                    world_block_set_color(world, blockLocation, block_color(block));
                }
            }
        }
    }
}
