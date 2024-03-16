#include "world_copy_chunk_command.h"

WorldCopyChunkCommand* world_copy_chunk_command_init(World* world, Chunk* chunk, int* location, int rotation) {
    WorldCopyChunkCommand* command = NEW(WorldCopyChunkCommand, 1);

    command->command.parent = command;
    command->command.undo = world_copy_chunk_command_undo;
    command->command.redo = world_copy_chunk_command_redo;
    command->command.id = world_copy_chunk_command_id;
    command->command.destroy = world_copy_chunk_command_destroy;

    command->world = world;
    command->chunk = chunk;
    memcpy(command->toLocation, location, sizeof(command->toLocation));

    command->chunkBefore = NULL;

    command->rotation = rotation;

    return command;
}

void world_copy_chunk_command_redo(Command* command) {
    WorldCopyChunkCommand* worldCopyChunkCommand = (WorldCopyChunkCommand*)command->parent;

    Box regionBefore;
    box_init(&regionBefore, worldCopyChunkCommand->world->vulkan);

    regionBefore.width = worldCopyChunkCommand->chunk->width;
    regionBefore.height = worldCopyChunkCommand->chunk->height;
    regionBefore.length = worldCopyChunkCommand->chunk->length;

    regionBefore.position[0] = worldCopyChunkCommand->toLocation[0];
    regionBefore.position[1] = worldCopyChunkCommand->toLocation[1];
    regionBefore.position[2] = worldCopyChunkCommand->toLocation[2];

    if (worldCopyChunkCommand->rotation % 2 == 1) {
        float temp = regionBefore.width;
        regionBefore.width = regionBefore.length;
        regionBefore.length = temp;
    }

    switch (worldCopyChunkCommand->rotation) {
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

    worldCopyChunkCommand->chunkBefore = world_copy_chunk(worldCopyChunkCommand->world, &regionBefore);

    world_set_chunk(worldCopyChunkCommand->world, worldCopyChunkCommand->chunk, worldCopyChunkCommand->toLocation, worldCopyChunkCommand->rotation);
}

void world_copy_chunk_command_undo(Command* command) {
    WorldCopyChunkCommand* worldCopyChunkCommand = (WorldCopyChunkCommand*)command->parent;

    int location[] = {
        worldCopyChunkCommand->toLocation[0],
        worldCopyChunkCommand->toLocation[1],
        worldCopyChunkCommand->toLocation[2]
    };

    switch (worldCopyChunkCommand->rotation) {
        case 0:
            break;
        case 1:
            location[2] -= worldCopyChunkCommand->chunkBefore->length - 1;
            break;
        case 2:
            location[0] -= worldCopyChunkCommand->chunkBefore->width - 1;
            location[2] -= worldCopyChunkCommand->chunkBefore->length - 1;
            break;
        case 3:
            location[0] -= worldCopyChunkCommand->chunkBefore->width - 1;
            break;
        default:
            break;
    }

    world_set_chunk(worldCopyChunkCommand->world, worldCopyChunkCommand->chunkBefore, location, 0);
}

int world_copy_chunk_command_id() {
    return 0;
}

void world_copy_chunk_command_destroy(Command* command) {
    WorldCopyChunkCommand* worldCopyChunkCommand = (WorldCopyChunkCommand*)command->parent;

    if (worldCopyChunkCommand->chunkBefore) {
        chunk_destroy(worldCopyChunkCommand->chunkBefore);
        free(worldCopyChunkCommand->chunkBefore);
    }

    if (worldCopyChunkCommand->chunk) {
        chunk_destroy(worldCopyChunkCommand->chunk);
        free(worldCopyChunkCommand->chunk);
    }
}