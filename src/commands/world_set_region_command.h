#ifndef WORLD_SET_REGION_COMMAND
#define WORLD_SET_REGION_COMMAND

#include "../undo_stack.h"
#include "../box.h"
#include "../world.h"

typedef struct {
    Command command;
    World* world;
    Box region;
    uint16_t color;
    Chunk* chunk;
} WorldSetRegionCommand;

WorldSetRegionCommand* world_set_region_command_init(World* world, Box* region, uint16_t color);

void world_set_region_command_undo(Command* command);
void world_set_region_command_redo(Command* command);
int world_set_region_command_id();
void world_set_region_command_destroy(Command* command);

#endif // WORLD_SET_REGION_COMMAND