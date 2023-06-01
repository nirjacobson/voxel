#ifndef BOX_H
#define BOX_H

#include <stdio.h>

#include "global.h"
#include "matrix.h"
#include "mesh.h"

typedef struct {
    float position[3];

    float forward[3];
    float up[3];
    float right[3];

    float width;
    float height;
    float length;
} Box;

Box* box_init(Box* b);

Box* box_aabb(Box* box);

Mesh* box_mesh(Box* box);

void box_print(Box* box);

#endif // BOX_H
