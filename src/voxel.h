#ifndef VOXEL_H
#define VOXEL_H

#include <sys/time.h>

#include "global.h"
#include "application.h"
#include "window.h"
#include "keyboard.h"
#include "mouse.h"

struct Voxel {
    Application application;
    Window window;

    Keyboard keyboard;
    Mouse mouse;
};

typedef struct Voxel Voxel;

Voxel* voxel_init(Voxel* v);
void voxel_destroy(Voxel* voxel);

void voxel_run(Voxel* voxel);

#endif // VOXEL_H
