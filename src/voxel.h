#ifndef VOXEL_H
#define VOXEL_H

#include <sys/time.h>

#include "global.h"
#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "world.h"
#include "window.h"
#include "application.h"

struct Voxel {
    Application application;
    Window window;

    Renderer renderer;

    World world;
    Camera camera;

    struct timeval frameTime;
};

typedef struct Voxel Voxel;

Voxel* voxel_init(Voxel* v);
void voxel_destroy(Voxel* voxel);

void voxel_run(Voxel* voxel);

#endif // VOXEL_H
