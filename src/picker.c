#include "picker.h"

/* Linked list processing callbacks */

void scale_vertices_up(void* ptr, void* unused) {
    Quad* quad = (Quad*)ptr;
    vec3_scale(quad->vertices[0].position, quad->vertices[0].position, 1.05);
    vec3_scale(quad->vertices[1].position, quad->vertices[1].position, 1.05);
    vec3_scale(quad->vertices[2].position, quad->vertices[2].position, 1.05);
    vec3_scale(quad->vertices[3].position, quad->vertices[3].position, 1.05);
}

/* Picker */

void picker_init(Picker* p) {
    Picker* picker = p ? p : NEW(Picker, 1);

    box_init(&picker->box);
    picker->mesh = box_mesh(&picker->box);
    linked_list_foreach(&picker->mesh->quads, scale_vertices_up, NULL);

    picker->selection.mesh = NULL;
    picker->selection.model = NULL;
    picker->selection.rotation = 0;
    picker->selection.present = 0;

    picker->dragging = 0;
    picker->mode = PICKER_ADJACENT;
    picker->action = PICKER_SET;

    picker->color = 0x01E4;
}

void picker_destroy(Picker* picker) {
    mesh_destroy(picker->mesh);
}

void picker_mode(Picker* picker, char mode) {
    picker->mode = mode;
}

void picker_press(Picker* picker, char modifier1, char modifier2) {
    picker->dragging = 1;
}

Box picker_merge_selections(Box* selectionA, Box* selectionB) {
    Box merged;
    box_init(&merged);
    
    merged.position[0] = MIN(selectionA->position[0], selectionB->position[0]);
    merged.position[1] = MIN(selectionA->position[1], selectionB->position[1]);
    merged.position[2] = MIN(selectionA->position[2], selectionB->position[2]);

    GLfloat endpointA[] = {
        selectionA->position[0] + selectionA->width,
        selectionA->position[1] + selectionA->height,
        selectionA->position[2] + selectionA->length
    };
    GLfloat endpointB[] = {
        selectionB->position[0] + selectionB->width,
        selectionB->position[1] + selectionB->height,
        selectionB->position[2] + selectionB->length
    };
    GLfloat mergedEndpoint[] = {
        MAX(endpointA[0], endpointB[0]),
        MAX(endpointA[1], endpointB[1]),
        MAX(endpointA[2], endpointB[2])
    };

    merged.width = mergedEndpoint[0] - merged.position[0];
    merged.height = mergedEndpoint[1] - merged.position[1];
    merged.length = mergedEndpoint[2] - merged.position[2];

    return merged;
}

void picker_act(Picker* picker, char modifier1, char modifier2) {
    if (picker->action == PICKER_SELECT) {
        picker->selection.box = modifier1 ? picker_merge_selections(&picker->selection.box, &picker->box) : picker->box;
        picker->selection.mesh = box_mesh(&picker->selection.box);
        picker->selection.present = 1;
        picker->selection.rotation = 0;
    } else if (picker->action == PICKER_STAMP || picker->action == PICKER_MOVE) {
        world_set_chunk(picker->world, picker->selection.model, picker->positionEnd, picker->selection.rotation);
        if (picker->action == PICKER_MOVE) {
            picker_set_action(picker, PICKER_SELECT);
        }
    } else {
        picker->selection.present = 0;
        if (picker->action == PICKER_EYEDROPPER) {
            Block* block = world_get_block(picker->world, picker->positionEnd);
            picker->color = block_color(block);
        } else if (picker->action == PICKER_SET || picker->action == PICKER_CLEAR) {
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
}

void picker_release(Picker* picker, char modifier1, char modifier2) {
    picker->dragging = 0;

    picker_act(picker, modifier1, modifier2);
    
    memcpy(picker->positionStart, picker->positionEnd, 3*sizeof(int));
    memcpy(picker->box.position, picker->positionEnd, 3*sizeof(int));

    picker->box.position[0] = (float)picker->positionEnd[0];
    picker->box.position[1] = (float)picker->positionEnd[1];
    picker->box.position[2] = (float)picker->positionEnd[2];
    picker->box.width = 1;
    picker->box.height = 1;
    picker->box.length = 1;

    mesh_destroy(picker->mesh);
    picker->mesh = box_mesh(&picker->box);
}

void picker_set_action(Picker* picker, char action) {
    picker->action = action;

    if (action == PICKER_STAMP || action == PICKER_MOVE) {
        picker->selection.model = action == PICKER_STAMP
            ? world_copy_chunk(picker->world, &picker->selection.box)
            : world_cut_chunk(picker->world, &picker->selection.box);
        chunk_mesh(picker->selection.model);
        picker->mode = PICKER_ADJACENT;
    }  else {
        picker->selection.present = 0;
        if (picker->selection.model) {
            chunk_destroy(picker->selection.model);
            free(picker->selection.model);
            picker->selection.model = NULL;
        }
    }
}

void picker_update(Picker* picker, Camera* camera, GLfloat mouseX, GLfloat mouseY) {
    picker->ray[0] = mouseX;
    picker->ray[1] = mouseY;
    picker->ray[2] =  -1;
    picker->ray[3] = 1.0;

    vec4_transform(picker->ray, camera->mat_proj_inv, picker->ray);
    picker->ray[2] = -1;
    picker->ray[3] =  0;

    vec4_transform(picker->ray, camera->mat_model, picker->ray);
    vec4_transform(picker->ray, camera->mat_view, picker->ray);

    vec3_normalize(picker->ray, picker->ray);

    GLfloat ray[3];
    GLfloat ray_point[3];
    int block_location[3];
    int block_location_adjacent[3];
    memcpy(ray_point, camera->position, 3*sizeof(GLfloat));
    vec3_scale(ray, picker->ray, 0.2);
    Block* block;
    int i;
    for (i = 0; i < 1000; ++i) {
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
        if (!(picker->dragging && (picker->action == PICKER_SELECT || picker->action == PICKER_SET || picker->action == PICKER_CLEAR))) {
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
    
    mesh_destroy(picker->mesh);
    picker->mesh = box_mesh(&picker->box);

    if (picker->selection.model) {
        memcpy(picker->selection.box.position, picker->box.position, 3*sizeof(GLfloat));
        picker->selection.mesh = box_mesh(&picker->selection.box);
        picker->selection.present = 1;
    }

}

void picker_draw(Picker* picker, Renderer* renderer) {
    GLfloat mat[16];
    GLfloat vec[3];

    if (picker->selection.present) {
        mat4_rotate(mat, NULL, (M_PI/2) * picker->selection.rotation, Y);
        switch (picker->selection.rotation) {
            case 0:
                break;
            case 1:
                vec[0] = 0;
                vec[1] = 0;
                vec[2] = picker->selection.box.width;
                mat4_translate(mat, mat, vec);
                break;
            case 2:
                vec[0] = picker->selection.box.width;
                vec[1] = 0;
                vec[2] = picker->selection.box.length;
                mat4_translate(mat, mat, vec);
                break;
            case 3:
                vec[0] = picker->selection.box.length;
                vec[1] = 0;
                vec[2] = 0;
                mat4_translate(mat, mat, vec);
                break;
            default:
                break;
        }
        renderer_3D_update_model(renderer, mat);
        renderer_3D_update_world_position(renderer, picker->selection.box.position);
        renderer_3D_update_color(renderer, 0,255,255);
        mesh_draw(picker->selection.mesh, renderer, MESH_LINE);
    }

    if (picker->selection.model) {
        chunk_draw(picker->selection.model, renderer, picker->box.position);
    } else {
        renderer_3D_update_world_position(renderer, picker->box.position);
        renderer_3D_update_color(renderer, 255,255,0);
        mesh_draw(picker->mesh, renderer, MESH_LINE);
    }

    mat4_identity(mat);
    renderer_3D_update_model(renderer, mat);
}

void picker_set_world(Picker* picker, World* world) {
    picker->world = world;
}

