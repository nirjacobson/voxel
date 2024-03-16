#ifndef WORLD_COPY_CHUNK_COMMAND
#define WORLD_COPY_CHUNK_COMMAND

#include "../undo_stack.h"
#include "../box.h"
#include "../world.h"

typedef struct {
    Command command;
    World* world;
    Chunk* chunk;
    Chunk* chunkBefore;
    int toLocation[3];
    int rotation;
} WorldCopyChunkCommand;

WorldCopyChunkCommand* world_copy_chunk_command_init(World* world, Chunk* chunk, int* location, int rotation);

void world_copy_chunk_command_undo(Command* command);
void world_copy_chunk_command_redo(Command* command);
int world_copy_chunk_command_id();
void world_copy_chunk_command_destroy(Command* command);

#endif // WORLD_COPY_CHUNK_COMMAND