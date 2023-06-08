#include "picker.h"
#include "internal/picker.h"

/* Linked list processing callbacks */

void scale_vertices_up(void* ptr, void* unused) {
    Quad* quad = (Quad*)ptr;
    vec3_scale(quad->vertices[0].position, quad->vertices[0].position, 1.05);
    vec3_scale(quad->vertices[1].position, quad->vertices[1].position, 1.05);
    vec3_scale(quad->vertices[2].position, quad->vertices[2].position, 1.05);
    vec3_scale(quad->vertices[3].position, quad->vertices[3].position, 1.05);
}

/* Picker */

void picker_init(Picker* p, World* world) {
    Picker* picker = p ? p : NEW(Picker, 1);
    picker->world = world;

    box_init(&picker->box);
    box_mesh(&picker->mesh, &picker->box);
    linked_list_foreach(&picker->mesh.quads, scale_vertices_up, NULL);

    picker->dragging = 0;
    picker->mode = PICKER_ADJACENT;
    picker->action = PICKER_SET;

    picker->color = 0x01E4;
}

void picker_destroy(Picker* picker) {
    mesh_destroy(&picker->mesh);
}

void picker_update(Picker* picker, Camera* camera, float mouseX, float mouseY) {
    float ray[4];
    ray[0] = mouseX;
    ray[1] = mouseY;
    ray[2] =   1;
    ray[3] = 1.0;

    // undivide by W
    vec4_scale(ray, ray, camera->near);

    vec4_transform(ray, camera->mat_proj_inv, ray);
    ray[2] = -camera->near; // for accuracy
    ray[3] =  0;

    vec4_transform(ray, camera->mat_model, ray);
    vec4_transform(ray, camera->mat_view, ray);

    vec3_normalize(ray, ray);
    vec3_scale(ray, ray, 0.2);

    float ray_point[3];
    memcpy(ray_point, camera->position, 3*sizeof(float));

    Block* block;
    int block_location[3];
    int block_location_adjacent[3];
    for (int i = 0; i < 1000; ++i) {
        memcpy(block_location_adjacent, block_location, 3*sizeof(int));
        block_location[0] = floor(ray_point[0]);
        block_location[1] = floor(ray_point[1]);
        block_location[2] = floor(ray_point[2]);

        block = world_get_block(picker->world, block_location);

        if (block_location[1] < 0)
            break;

        if (block && block_is_active(block))
            break;

        vec3_add(ray_point, ray_point, ray);
    }

    if (block || block_location[1] == -1) {
        if (!(picker->dragging && (picker->action == PICKER_SET || picker->action == PICKER_CLEAR))) {
            if (picker->mode == PICKER_ONTO && block_location[1] >= 0) {
                memcpy(picker->positionStart, block_location, 3*sizeof(int));
            } else {
                memcpy(picker->positionStart, block_location_adjacent, 3*sizeof(int));
            }
        }
        if (picker->mode == PICKER_ONTO && block_location[1] >= 0) {
            memcpy(picker->positionEnd, block_location, 3*sizeof(int));
        } else {
            memcpy(picker->positionEnd, block_location_adjacent, 3*sizeof(int));
        }
    }

    picker->box.position[0] = MIN(picker->positionStart[0], picker->positionEnd[0]);
    picker->box.position[1] = MIN(picker->positionStart[1], picker->positionEnd[1]);
    picker->box.position[2] = MIN(picker->positionStart[2],  picker->positionEnd[2]);
    picker->box.width = abs(picker->positionEnd[0] - picker->positionStart[0]) + 1;
    picker->box.height = abs(picker->positionEnd[1] - picker->positionStart[1]) + 1;
    picker->box.length = abs(picker->positionEnd[2] - picker->positionStart[2]) + 1;

    mesh_destroy(&picker->mesh);
    box_mesh(&picker->mesh, &picker->box);
}

void picker_press(Picker* picker, char modifier1, char modifier2) {
    picker->dragging = 1;
}

void picker_release(Picker* picker, char modifier1, char modifier2) {
    picker->dragging = 0;

    picker_act(picker, modifier1, modifier2);

    memcpy(picker->positionStart, picker->positionEnd, 3*sizeof(int));

    picker->box.position[0] = picker->positionEnd[0];
    picker->box.position[1] = picker->positionEnd[1];
    picker->box.position[2] = picker->positionEnd[2];
    picker->box.width = 1;
    picker->box.height = 1;
    picker->box.length = 1;

    mesh_destroy(&picker->mesh);
    box_mesh(&picker->mesh, &picker->box);
}

void picker_act(Picker* picker, char modifier1, char modifier2) {
    if (picker->action == PICKER_SET || picker->action == PICKER_CLEAR) {
        for (int x = 0; x < picker->box.width; x++) {
            for (int y = 0; y < picker->box.height; y++) {
                for (int z = 0; z < picker->box.length; z++) {
                    int location[3] = {
                        picker->box.position[0] + x,
                        picker->box.position[1] + y,
                        picker->box.position[2] + z
                    };
                    switch (picker->action) {
                        case PICKER_CLEAR:
                            world_block_set_active(picker->world, location, 0);
                            break;
                        case PICKER_SET:
                            world_block_set_color(picker->world, location, picker->color);
                            world_block_set_active(picker->world, location, 1);
                            break;
                    }
                }
            }
        }
    }
}

void picker_set_action(Picker* picker, char action) {
    picker->action = action;
}
