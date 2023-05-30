#ifndef VOXEL_INTERNAL_H
#define VOXEL_INTERNAL_H

#include "../voxel.h"

char voxel_process_input(Voxel* voxel);
void voxel_draw(Voxel* voxel);

void voxel_setup(Application* application);
void voxel_main(Application* application);
void voxel_resize(Application* application);
void voxel_teardown(Application* application);

#endif // VOXEL_INTERNAL_H
