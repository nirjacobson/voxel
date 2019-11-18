#include "ground.h"

Ground* ground_init(Ground* g, int length) {
    Ground* ground = g ? g : NEW(Ground, 1);

    mesh_init(&ground->mesh);

    Quad* quad = NEW(Quad, 1);

    float backLeft[] = {
        -length/2, 0, -length/2
    };
    float frontLeft[] = {
        -length/2, 0, length/2
    };
    float backRight[] = {
        length/2, 0, -length/2
    };
    float frontRight[] = {
        length/2, 0, length/2
    };
    memcpy(quad->vertices[0].position, backLeft, sizeof(backLeft));
    memcpy(quad->vertices[1].position, frontLeft, sizeof(frontLeft));
    memcpy(quad->vertices[2].position, backRight, sizeof(backRight));
    memcpy(quad->vertices[3].position, frontRight, sizeof(frontRight));
    for (int i = 0; i < 4; i++)
        quad->vt_order[i] = i;
    
    quad->orientation = TOP;

    mesh_add_quad(&ground->mesh, quad);
    mesh_calc_normals(&ground->mesh);
    mesh_buffer(&ground->mesh, MESH_FILL);

    return ground;
}

void ground_destroy(Ground* ground) {
    mesh_destroy(&ground->mesh);
}

void ground_draw(Ground* ground, Camera* camera, Renderer* renderer) {
    float worldPosition[] = {
        camera->mat_view[12],
        0,
        camera->mat_view[14],
    };
    renderer_3D_update_world_position(renderer, worldPosition);
    renderer_3D_update_color(renderer, 192, 192, 192);
    mesh_draw(&ground->mesh, renderer, MESH_FILL);
}