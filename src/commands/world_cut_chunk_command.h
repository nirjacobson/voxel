#ifndef WORLD_CUT_CHUNK_COMMAND
#define WORLD_CUT_CHUNK_COMMAND

#include "../undo_stack.h"
#include "../box.h"
#include "../world.h"

typedef struct {
    Command command;
    World* world;
    Chunk* chunk;
    Chunk* chunkBefore;
    int fromLocation[3];
    int toLocation[3];
    int rotation;
} WorldCutChunkCommand;

WorldCutChunkCommand* world_cut_chunk_command_init(World* world, Chunk* chunk, int* fromLocation, int* toLocation, int rotation);

void world_cut_chunk_command_undo(Command* command);
void world_cut_chunk_command_redo(Command* command);
int world_cut_chunk_command_id();
void world_cut_chunk_command_destroy(Command* command);

#endif // WORLD_CUT_CHUNK_COMMAND