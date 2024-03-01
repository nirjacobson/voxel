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
    camera->forward[2] = 1.0;

    camera->up[0] = 0.0;
    camera->up[1] = 1.0;
    camera->up[2] = 0.0;

    camera->right[0] =  1.0;
    camera->right[1] =  0.0;
    camera->right[2] =  0.0;

    camera_update_projection(camera);

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

void camera_set_aspect(Camera* camera, float aspect) {
    camera->aspect = aspect;
}

Box* camera_aabb(Box* box, Camera* camera) {
    Box frustum_bb;
    box_init(&frustum_bb, NULL);

    frustum_bb.height = 2 * tan(camera->fov/2.0 * M_PI/180.0) * camera->far;
    frustum_bb.width = frustum_bb.height * camera->aspect;
    frustum_bb.length = camera->far;

    vec3_transform(frustum_bb.position, camera->mat_model, frustum_bb.position);
    vec3_transform(frustum_bb.position, camera->mat_view, frustum_bb.position);

    memcpy(frustum_bb.forward, camera->forward, 3*sizeof(float));
    memcpy(frustum_bb.up, camera->up, 3*sizeof(float));
    memcpy(frustum_bb.right, camera->right, 3*sizeof(float));

    float shift_left[3];
    float shift_down[3];

    vec3_scale(shift_left, frustum_bb.right, -frustum_bb.width / 2.0f);
    vec3_scale(shift_down, frustum_bb.up, -frustum_bb.height / 2.0f);

    float mat[16];

    mat4_translate(mat, NULL, shift_left);
    mat4_translate(mat, mat, shift_down);

    vec3_transform(frustum_bb.position, mat, frustum_bb.position);

    Box* frustum_aabb = box_aabb(box, &frustum_bb);

    return frustum_aabb;
}

void camera_update_projection(Camera* camera) {
    mat4_perspective(camera->mat_proj, camera->fov, camera->aspect, camera->near, camera->far);
    mat4_inverse(camera->mat_proj_inv, camera->mat_proj);
}