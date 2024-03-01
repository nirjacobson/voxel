#include "ground.h"

Ground* ground_init(Ground* g, Vulkan* vulkan, int length) {
    Ground* ground = g ? g : NEW(Ground, 1);

    mesh_init(&ground->mesh, vulkan);

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
    memcpy(quad->vertices[1].position, backRight, sizeof(backRight));
    memcpy(quad->vertices[2].position, frontLeft, sizeof(frontLeft));
    memcpy(quad->vertices[3].position, frontRight, sizeof(frontRight));

    quad->orientation = TOP;

    mesh_add_quad(&ground->mesh, quad);
    mesh_calc_normals(&ground->mesh);
    mesh_buffer(&ground->mesh, MESH_FILL);

    return ground;
}

void ground_destroy(Ground* ground) {
    mesh_destroy(&ground->mesh);
}
