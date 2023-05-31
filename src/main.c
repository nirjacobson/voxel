#include "voxel.h"

int main(int argc, char** argv) {
    Voxel voxel;

    voxel_init(&voxel);

    voxel_run(&voxel);

    voxel_destroy(&voxel);
}