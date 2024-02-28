#ifndef BOX_H
#define BOX_H

#include <stdio.h>

#include "global.h"
#include "matrix.h"
#include "mesh.h"

typedef struct {
    Vulkan* vulkan;
    float position[3];

    float forward[3];
    float up[3];
    float right[3];

    float width;
    float height;
    float length;
} Box;

Box* box_init(Box* b, Vulkan* vulkan);

Box* box_aabb(Box* boxd, Box* boxs);

Mesh* box_mesh(Mesh* mesh, Box* box);

void box_print(Box* box);

#endif // BOX_H
