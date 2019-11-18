#ifndef GROUND_H
#define GROUND_H

#include "global.h"
#include "mesh.h"
#include "renderer.h"
#include "camera.h"

typedef struct {
    Mesh mesh;
} Ground;

Ground* ground_init(Ground* g, int length);
void ground_destroy(Ground* ground);

void ground_draw(Ground* ground, Camera* camera, Renderer* renderer);

#endif // GROUND_H
