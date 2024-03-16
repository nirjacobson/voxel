#include "world_clear_region_command.h"

WorldClearRegionCommand* world_clear_region_command_init(World* world, Box* region) {
    WorldClearRegionCommand* command = NEW(WorldClearRegionCommand, 1);

    command->command.parent = command;
    command->command.undo = world_clear_region_command_undo;
    command->command.redo = world_clear_region_command_redo;
    command->command.id = world_clear_region_command_id;
    command->command.destroy = world_clear_region_command_destroy;

    command->world = world;
    command->region = *region;
    command->chunk = NULL;

    return command;
}

void world_clear_region_command_redo(Command* command) {
    WorldClearRegionCommand* worldClearRegionCommand = (WorldClearRegionCommand*)command->parent;

    worldClearRegionCommand->chunk = world_copy_chunk(worldClearRegionCommand->world, &worldClearRegionCommand->region);

    for (int x = 0; x < worldClearRegionCommand->region.width; x++) {
        for (int y = 0; y < worldClearRegionCommand->region.height; y++) {
            for (int z = 0; z < worldClearRegionCommand->region.length; z++) {
                int location[3] = {
                    worldClearRegionCommand->region.position[0] + x,
                    worldClearRegionCommand->region.position[1] + y,
                    worldClearRegionCommand->region.position[2] + z
                };
                world_block_set_active(worldClearRegionCommand->world, location, 0);
            }
        }
    }
}

void world_clear_region_command_undo(Command* command) {
    WorldClearRegionCommand* worldClearRegionCommand = (WorldClearRegionCommand*)command->parent;
    int position[3] = {
        worldClearRegionCommand->region.position[0],
        worldClearRegionCommand->region.position[1],
        worldClearRegionCommand->region.position[2]
    };

    world_set_chunk(worldClearRegionCommand->world, worldClearRegionCommand->chunk, position, 0);
}

int world_clear_region_command_id() {
    return 0;
}

void world_clear_region_command_destroy(Command* command) {
    WorldClearRegionCommand* worldClearRegionCommand = (WorldClearRegionCommand*)command->parent;

    if (worldClearRegionCommand->chunk) {
        chunk_destroy(worldClearRegionCommand->chunk);
        free(worldClearRegionCommand->chunk);
    }
}