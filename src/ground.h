#ifndef GROUND_H
#define GROUND_H

#include <string.h>

#include "global.h"
#include "mesh.h"
#include "camera.h"

typedef struct {
    Mesh mesh;
} Ground;

Ground* ground_init(Ground* g, int length);
void ground_destroy(Ground* ground);

#endif // GROUND_H
