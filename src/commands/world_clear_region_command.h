#ifndef WORLD_CLEAR_REGION_COMMAND_H
#define WORLD_CLEAR_REGION_COMMAND_H

#include "../undo_stack.h"
#include "../box.h"
#include "../world.h"

typedef struct {
    Command command;
    World* world;
    Box region;
    Chunk* chunk;
} WorldClearRegionCommand;

WorldClearRegionCommand* world_clear_region_command_init(World* world, Box* region);

void world_clear_region_command_undo(Command* command);
void world_clear_region_command_redo(Command* command);
int world_clear_region_command_id();
void world_clear_region_command_destroy(Command* command);

#endif // WORLD_CLEAR_REGION_COMMAND_H