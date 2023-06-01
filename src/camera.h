#ifndef CAMERA_H
#define CAMERA_H

#include <stdlib.h>
#include <math.h>

#include "box.h"

typedef struct {
    double fov;
    double aspect;
    double near;
    double far;

    float position[3];
    float forward[3];
    float up[3];
    float right[3];

    float mat_model[16];
    float mat_view[16];
    float mat_proj[16];
    float mat_proj_inv[16];
} Camera;

Camera* camera_init(Camera* c);

void camera_rotate(Camera* camera, float* axis, float radians);
void camera_move(Camera* camera, float* direction, float amount);
void camera_set_aspect(Camera* camera, double aspect);

Box* camera_aabb();

#endif // CAMERA_H
