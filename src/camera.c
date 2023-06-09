#include "camera.h"

Camera* camera_init(Camera* c) {
    Camera* camera = c ? c : NEW(Camera, 1);

    camera->fov = 60;
    camera->aspect = 1;
    camera->near = 1.0;
    camera->far = 100.0;

    mat4_identity(camera->mat_model);
    mat4_identity(camera->mat_view);

    camera->position[0] = 0;
    camera->position[1] = 0;
    camera->position[2] = 0;

    camera->forward[0] =  0.0;
    camera->forward[1] =  0.0;
    camera->forward[2] = -1.0;

    camera->up[0] = 0.0;
    camera->up[1] = 1.0;
    camera->up[2] = 0.0;

    camera->right[0] =  1.0;
    camera->right[1] =  0.0;
    camera->right[2] =  0.0;

    return camera;
}

void camera_rotate(Camera* camera, float* axis, float radians) {
    float mat[16];

    mat4_rotate(mat, NULL, radians, axis);
    mat4_multiply(camera->mat_model, mat, camera->mat_model);

    vec3_transform(camera->forward, mat, camera->forward);
    vec3_transform(camera->up, mat, camera->up);
    vec3_transform(camera->right, mat, camera->right);
}

void camera_move(Camera* camera, float* direction, float amount) {
    float vec[3];
    vec3_scale(vec, direction, amount);

    vec3_add(camera->position, camera->position, vec);

    mat4_translate(camera->mat_view, NULL, camera->position);
}

void camera_set_aspect(Camera* camera, double aspect) {
    camera->aspect = aspect;
}
