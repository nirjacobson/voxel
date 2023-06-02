#ifndef WORLD_H
#define WORLD_H

#define WORLD_CHUNK_LENGTH    16

#include <stdlib.h>

#include "camera.h"
#include "ground.h"

typedef struct {
    Ground ground;
} World;

World* world_init(World* world, const char* name);
void world_destroy(World* world);

#endif // WORLD_H
