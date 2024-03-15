#include "world_cut_chunk_command.h"

WorldCutChunkCommand* world_cut_chunk_command_init(World* world, Chunk* chunk, int* fromLocation, int* toLocation, int rotation) {
    WorldCutChunkCommand* command = NEW(WorldCutChunkCommand, 1);

    command->command.parent = command;
    command->command.undo = world_cut_chunk_command_undo;
    command->command.redo = world_cut_chunk_command_redo;
    command->command.id = world_cut_chunk_command_id;
    command->command.destroy = world_cut_chunk_command_destroy;

    command->world = world;
    command->chunk = chunk;
    memcpy(command->fromLocation, fromLocation, sizeof(command->fromLocation));
    memcpy(command->toLocation, toLocation, sizeof(command->toLocation));

    command->chunkBefore = NULL;

    command->rotation = rotation;

    return command;
}

void world_cut_chunk_command_redo(Command* command) {
    WorldCutChunkCommand* worldCutChunkCommand = (WorldCutChunkCommand*)command->parent;

    Box regionBefore;
    box_init(&regionBefore);

    regionBefore.width = worldCutChunkCommand->chunk->width;
    regionBefore.height = worldCutChunkCommand->chunk->height;
    regionBefore.length = worldCutChunkCommand->chunk->length;

    regionBefore.position[0] = worldCutChunkCommand->toLocation[0];
    regionBefore.position[1] = worldCutChunkCommand->toLocation[1];
    regionBefore.position[2] = worldCutChunkCommand->toLocation[2];

    if (worldCutChunkCommand->rotation % 2 == 1) {
        float temp = regionBefore.width;
        regionBefore.width = regionBefore.length;
        regionBefore.length = temp;
    }

    switch (worldCutChunkCommand->rotation) {
        case 0:
            break;
        case 1:
            regionBefore.position[2] -= regionBefore.length - 1;
            break;
        case 2:
            regionBefore.position[0] -= regionBefore.width - 1;
            regionBefore.position[2] -= regionBefore.length - 1;
            break;
        case 3:
            regionBefore.position[0] -= regionBefore.width - 1;
            break;
        default:
            break;
    }

    Box regionSource;
    box_init(&regionSource);

    regionSource.width = worldCutChunkCommand->chunk->width;
    regionSource.height = worldCutChunkCommand->chunk->height;
    regionSource.length = worldCutChunkCommand->chunk->length;

    regionSource.position[0] = worldCutChunkCommand->fromLocation[0];
    regionSource.position[1] = worldCutChunkCommand->fromLocation[1];
    regionSource.position[2] = worldCutChunkCommand->fromLocation[2];


    worldCutChunkCommand->chunkBefore = world_copy_chunk(worldCutChunkCommand->world, &regionBefore);

    world_clear_region(worldCutChunkCommand->world, &regionSource);
    world_set_chunk(worldCutChunkCommand->world, worldCutChunkCommand->chunk, worldCutChunkCommand->toLocation, worldCutChunkCommand->rotation);
}

void world_cut_chunk_command_undo(Command* command) {
    WorldCutChunkCommand* worldCutChunkCommand = (WorldCutChunkCommand*)command->parent;

    int location[] = {
        worldCutChunkCommand->toLocation[0],
        worldCutChunkCommand->toLocation[1],
        worldCutChunkCommand->toLocation[2]
    };

    switch (worldCutChunkCommand->rotation) {
        case 0:
            break;
        case 1:
            location[2] -= worldCutChunkCommand->chunkBefore->length - 1;
            break;
        case 2:
            location[0] -= worldCutChunkCommand->chunkBefore->width - 1;
            location[2] -= worldCutChunkCommand->chunkBefore->length - 1;
            break;
        case 3:
            location[0] -= worldCutChunkCommand->chunkBefore->width - 1;
            break;
        default:
            break;
    }

    world_set_chunk(worldCutChunkCommand->world, worldCutChunkCommand->chunkBefore, location, 0);
    world_set_chunk(worldCutChunkCommand->world, worldCutChunkCommand->chunk, worldCutChunkCommand->fromLocation, 0);
}

int world_cut_chunk_command_id() {
    return 0;
}

void world_cut_chunk_command_destroy(Command* command) {
    WorldCutChunkCommand* worldCutChunkCommand = (WorldCutChunkCommand*)command->parent;

    if (worldCutChunkCommand->chunkBefore) {
        chunk_destroy(worldCutChunkCommand->chunkBefore);
        free(worldCutChunkCommand->chunkBefore);
    }

    if (worldCutChunkCommand->chunk) {
        chunk_destroy(worldCutChunkCommand->chunk);
        free(worldCutChunkCommand->chunk);
    }
}