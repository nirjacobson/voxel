#ifndef CAMERA_H
#define CAMERA_H

#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "matrix.h"
#include "box.h"

typedef struct {
    float fov;
    float aspect;
    float near;
    float far;

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
void camera_set_aspect(Camera* camera, float aspect);

Box* camera_aabb(Box* box, Camera* camera);

#endif // CAMERA_H
