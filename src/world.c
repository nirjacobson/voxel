#include "world.h"
#include "internal/world.h"

/* Linked list processing callbacks */

void load_world_chunk(void* chunkIDPtr, void* worldPtr) {
    ChunkID* chunkID = (ChunkID*)chunkIDPtr;
    World* world = (World*)worldPtr;

    world_load_world_chunk(world, chunkID);
}

void unload_world_chunk(void* worldChunkPtr, void* worldPtr) {
    WorldChunk* worldChunk = (WorldChunk*)worldChunkPtr;
    World* world = (World*)worldPtr;

    world_unload_world_chunk(world, worldChunk);
}

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

    chunk_dao_init(&w->chunkDAO, name);

    linked_list_init(&w->chunks);

    ground_init(&w->ground, 500);

    return w;
}

void world_destroy(World* world) {
    ground_destroy(&world->ground);
    linked_list_foreach(&world->chunks, unload_world_chunk, world);
    chunk_dao_destroy(&world->chunkDAO);
}

LinkedList* world_draw_list(Camera* camera) {
    Box aabb;
    camera_aabb(&aabb, camera);

    ChunkID chunkIDStart, chunkIDEnd;
    chunkIDStart.x = floor(aabb.position[0] / WORLD_CHUNK_LENGTH);
    chunkIDStart.y = floor(aabb.position[1] / WORLD_CHUNK_LENGTH);
    chunkIDStart.z = floor(aabb.position[2] / WORLD_CHUNK_LENGTH);
    memcpy(&chunkIDEnd, &chunkIDStart, sizeof(ChunkID));

    int block_position[3] = { chunkIDStart.x * WORLD_CHUNK_LENGTH,
                              chunkIDStart.y * WORLD_CHUNK_LENGTH,
                              chunkIDStart.z * WORLD_CHUNK_LENGTH
                            };
    float vec[3];

    vec3_scale(vec, aabb.right, aabb.width);
    vec3_add(vec, aabb.position, vec);
    while (block_position[0] < vec[0]) {
        chunkIDEnd.x++;
        block_position[0] += WORLD_CHUNK_LENGTH;
    }

    vec3_scale(vec, aabb.up, aabb.height);
    vec3_add(vec, aabb.position, vec);
    while (block_position[1] < vec[1]) {
        chunkIDEnd.y++;
        block_position[1] += WORLD_CHUNK_LENGTH;
    }

    vec3_scale(vec, aabb.forward, aabb.length);
    vec3_add(vec, aabb.position, vec);
    while (block_position[2] < vec[2]) {
        chunkIDEnd.z++;
        block_position[2] += WORLD_CHUNK_LENGTH;
    }

    LinkedList* drawList = linked_list_init(NULL);
    for (int x = chunkIDStart.x; x < chunkIDEnd.x; x++) {
        for (int y = chunkIDStart.y; y < chunkIDEnd.y; y++) {
            for (int z = chunkIDStart.z; z < chunkIDEnd.z; z++) {
                ChunkID* chunkID = NEW(ChunkID, 1);
                chunkID->x = x;
                chunkID->y = y;
                chunkID->z = z;

                linked_list_insert(drawList, chunkID);
            }
        }
    }

    return drawList;
}

Chunk* world_load_world_chunk(World* world, ChunkID* chunkID) {
    Chunk* chunk = chunk_dao_load(&world->chunkDAO, chunkID);
    if (chunk) {
        chunk_mesh(chunk);
        WorldChunk* worldChunk = NEW(WorldChunk, 1);
        worldChunk->id = *chunkID;
        worldChunk->chunk = chunk;
        linked_list_insert(&world->chunks, worldChunk);
    }
    return chunk;
}

void world_unload_world_chunk(World* world, WorldChunk* worldChunk) {
    if (worldChunk->chunk->dirty) {
        chunk_dao_save(&world->chunkDAO, &worldChunk->id, worldChunk->chunk);
    }

    LinkedListNode* node = linked_list_find(&world->chunks, &worldChunk->id, chunk_id_equals_world_chunk);
    linked_list_remove(&world->chunks, node, destroy_world_chunk);
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
    chunk->dirty = 1;
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
    chunk->dirty = 1;
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
                        blockLocation[2] = location[2] - x;
                        break;
                    case 2:
                        blockLocation[0] = location[0] - x;
                        blockLocation[2] = location[2] - z;
                        break;
                    case 3:
                        blockLocation[0] = location[0] - z;
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

void world_update(World* world, Camera* camera) {
    LinkedList* drawList = world_draw_list(camera);
    LinkedList chunksToUnload;
    LinkedList chunksToLoad;

    linked_list_init(&chunksToUnload);
    linked_list_init(&chunksToLoad);

    LinkedListNode* drawListNode = drawList->head;
    LinkedListNode* chunksListNode = world->chunks.head;

    while (drawListNode || chunksListNode) {
        if (!drawListNode) {
            WorldChunk* worldChunk = (WorldChunk*)chunksListNode->data;
            linked_list_insert(&chunksToUnload, worldChunk);
            chunksListNode = chunksListNode->next;
        } else if (!chunksListNode) {
            ChunkID* drawListChunkID = (ChunkID*)drawListNode->data;
            linked_list_insert(&chunksToLoad, drawListChunkID);
            drawListNode = drawListNode->next;
        } else {
            ChunkID* drawListChunkID = (ChunkID*)drawListNode->data;
            WorldChunk* worldChunk = (WorldChunk*)chunksListNode->data;
            int comparison = compare_chunk_ids(drawListChunkID, &worldChunk->id);

            if (comparison < 0) {
                linked_list_insert(&chunksToLoad, drawListChunkID);
                drawListNode = drawListNode->next;
            } else if (comparison == 0) {
                drawListNode = drawListNode->next;
                chunksListNode = chunksListNode->next;
            } else {
                linked_list_insert(&chunksToUnload, worldChunk);
                chunksListNode = chunksListNode->next;
            }
        }
    }

    linked_list_foreach(&chunksToUnload, unload_world_chunk, world);
    linked_list_foreach(&chunksToLoad, load_world_chunk, world);

    linked_list_destroy(&chunksToLoad, NULL);
    linked_list_destroy(&chunksToUnload, NULL);
}