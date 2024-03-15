#include "world_set_region_command.h"

WorldSetRegionCommand* world_set_region_command_init(World* world, Box* region, uint16_t color) {
    WorldSetRegionCommand* command = NEW(WorldSetRegionCommand, 1);

    command->command.parent = command;
    command->command.undo = world_set_region_command_undo;
    command->command.redo = world_set_region_command_redo;
    command->command.id = world_set_region_command_id;
    command->command.destroy = world_set_region_command_destroy;

    command->world = world;
    command->region = *region;
    command->color = color;
    command->chunk = NULL;

    return command;
}

void world_set_region_command_redo(Command* command) {
    WorldSetRegionCommand* worldSetRegionCommand = (WorldSetRegionCommand*)command->parent;

    worldSetRegionCommand->chunk = world_copy_chunk(worldSetRegionCommand->world, &worldSetRegionCommand->region);

    for (int x = 0; x < worldSetRegionCommand->region.width; x++) {
        for (int y = 0; y < worldSetRegionCommand->region.height; y++) {
            for (int z = 0; z < worldSetRegionCommand->region.length; z++) {
                int location[3] = {
                    worldSetRegionCommand->region.position[0] + x,
                    worldSetRegionCommand->region.position[1] + y,
                    worldSetRegionCommand->region.position[2] + z
                };
                world_block_set_color(worldSetRegionCommand->world, location, worldSetRegionCommand->color);
                world_block_set_active(worldSetRegionCommand->world, location, 1);
            }
        }
    }
}

void world_set_region_command_undo(Command* command) {
    WorldSetRegionCommand* worldSetRegionCommand = (WorldSetRegionCommand*)command->parent;
    int position[3] = {
        worldSetRegionCommand->region.position[0],
        worldSetRegionCommand->region.position[1],
        worldSetRegionCommand->region.position[2]
    };

    world_set_chunk(worldSetRegionCommand->world, worldSetRegionCommand->chunk, position, 0);
}

int world_set_region_command_id() {
    return 0;
}

void world_set_region_command_destroy(Command* command) {
    WorldSetRegionCommand* worldSetRegionCommand = (WorldSetRegionCommand*)command->parent;

    if (worldSetRegionCommand->chunk) {
        chunk_destroy(worldSetRegionCommand->chunk);
        free(worldSetRegionCommand->chunk);
    }
}