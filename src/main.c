#include <sys/time.h>

#include "keyboard.h"
#include "mouse.h"
#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "voxel.h"

int main(int argc, char** argv) {
    Voxel voxel;

    voxel_init(&voxel);

    voxel_run(&voxel);

    voxel_destroy(&voxel);
}