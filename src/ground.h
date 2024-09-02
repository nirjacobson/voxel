#ifndef GROUND_H
#define GROUND_H

#include <string.h>

#include "global.h"
#include "mesh.h"

typedef struct {
    Mesh mesh;
} Ground;

Ground* ground_init(Ground* g, Vulkan* vulkan, int length);
void ground_destroy(Ground* ground);

#endif // GROUND_H
