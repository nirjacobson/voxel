#include "box.h"

Box* box_init(Box* b) {
    Box* box = b ? b : NEW(Box, 1);

    box->position[0] = 0;
    box->position[1] = 0;
    box->position[2] = 0;

    box->forward[0] = 0;
    box->forward[1] = 0;
    box->forward[2] = 1;

    box->up[0] = 0;
    box->up[1] = 1;
    box->up[2] = 0;

    box->right[0] = 1;
    box->right[1] = 0;
    box->right[2] = 0;

    box->width = 1;
    box->height = 1;
    box->length = 1;

    return box;
}

Box* box_aabb(Box* boxd, Box* boxs) {
    float box_vertices[24];

    for (int i = 0; i < 24; i++) {
        box_vertices[i] = 0;
    }

    float forward[3];
    float up[3];
    float right[3];

    vec3_scale(forward, boxs->forward, boxs->length);
    vec3_scale(up, boxs->up, boxs->height);
    vec3_scale(right, boxs->right, boxs->width);

    vec3_add(&box_vertices[0], &box_vertices[0], boxs->position);
    vec3_add(&box_vertices[3], &box_vertices[0], forward);
    vec3_add(&box_vertices[6], &box_vertices[0], right);
    vec3_add(&box_vertices[9], &box_vertices[6], forward);

    vec3_add(&box_vertices[12], &box_vertices[0], up);
    vec3_add(&box_vertices[15], &box_vertices[12], forward);
    vec3_add(&box_vertices[18], &box_vertices[12], right);
    vec3_add(&box_vertices[21], &box_vertices[18], forward);

    float x[2];
    float y[2];
    float z[2];

    x[0] = box_vertices[0];
    y[0] = box_vertices[1];
    z[0] = box_vertices[2];

    x[1] = box_vertices[0];
    y[1] = box_vertices[1];
    z[1] = box_vertices[2];

    for (int i = 1; i < 8; ++i) {
        if (box_vertices[3*i+0] < x[0])
            x[0] = box_vertices[3*i+0];
        if (box_vertices[3*i+0] > x[1])
            x[1] = box_vertices[3*i+0];

        if (box_vertices[3*i+1] < y[0])
            y[0] = box_vertices[3*i+1];
        if (box_vertices[3*i+1] > y[1])
            y[1] = box_vertices[3*i+1];

        if (box_vertices[3*i+2] < z[0])
            z[0] = box_vertices[3*i+2];
        if (box_vertices[3*i+2] > z[1])
            z[1] = box_vertices[3*i+2];
    }

    Box* aabb = box_init(boxd);

    aabb->position[0] = x[0];
    aabb->position[1] = y[0];
    aabb->position[2] = z[0];

    aabb->width = x[1] - x[0];
    aabb->height = y[1] - y[0];
    aabb->length = z[1] - z[0];

    return aabb;
}

Mesh* box_mesh(Mesh* m, Box* box) {
    Mesh* mesh = mesh_init(m);

    Quad* leftQuad;
    Quad* topQuad;
    Quad* frontQuad;
    Quad* rightQuad;
    Quad* bottomQuad;
    Quad* backQuad;

    float mat[16];

    float forward[3];
    float up[3];
    float right[3];
    float zero[3] = { 0, 0, 0 };

    vec3_scale(forward, box->forward, box->length);
    vec3_scale(up, box->up, box->height);
    vec3_scale(right, box->right, box->width);

    leftQuad = NEW(Quad, 1);
    vec3_add(leftQuad->vertices[0].position, zero, up);
    vec3_add(leftQuad->vertices[1].position, zero, zero);
    vec3_add(leftQuad->vertices[2].position, zero, up);
    vec3_add(leftQuad->vertices[2].position, leftQuad->vertices[2].position, forward);
    vec3_add(leftQuad->vertices[3].position, zero, forward);
    leftQuad->orientation = WEST;
    mesh_add_quad(mesh, leftQuad);

    rightQuad = NEW(Quad, 1);
    mat4_rotate(mat, NULL, M_PI, Y);
    vec3_add(rightQuad->vertices[0].position, leftQuad->vertices[2].position, right);
    vec3_add(rightQuad->vertices[1].position, leftQuad->vertices[3].position, right);
    vec3_add(rightQuad->vertices[2].position, leftQuad->vertices[0].position, right);
    vec3_add(rightQuad->vertices[3].position, leftQuad->vertices[1].position, right);
    rightQuad->orientation = EAST;
    mesh_add_quad(mesh, rightQuad);

    backQuad = NEW(Quad, 1);
    vec3_add(backQuad->vertices[0].position, rightQuad->vertices[2].position, zero);
    vec3_add(backQuad->vertices[1].position, rightQuad->vertices[3].position, zero);
    vec3_add(backQuad->vertices[2].position, leftQuad->vertices[0].position, zero);
    vec3_add(backQuad->vertices[3].position, leftQuad->vertices[1].position, zero);
    backQuad->orientation = NORTH;
    mesh_add_quad(mesh, backQuad);

    frontQuad = NEW(Quad, 1);
    mat4_rotate(mat, NULL, M_PI, Y);
    vec3_add(frontQuad->vertices[0].position, backQuad->vertices[2].position, forward);
    vec3_add(frontQuad->vertices[1].position, backQuad->vertices[3].position, forward);
    vec3_add(frontQuad->vertices[2].position, backQuad->vertices[0].position, forward);
    vec3_add(frontQuad->vertices[3].position, backQuad->vertices[1].position, forward);
    frontQuad->orientation = SOUTH;
    mesh_add_quad(mesh, frontQuad);

    bottomQuad = NEW(Quad, 1);
    vec3_add(bottomQuad->vertices[0].position, leftQuad->vertices[3].position, zero);
    vec3_add(bottomQuad->vertices[1].position, leftQuad->vertices[1].position, zero);
    vec3_add(bottomQuad->vertices[2].position, rightQuad->vertices[1].position, zero);
    vec3_add(bottomQuad->vertices[3].position, rightQuad->vertices[3].position, zero);
    bottomQuad->orientation = BOTTOM;
    mesh_add_quad(mesh, bottomQuad);

    topQuad = NEW(Quad, 1);
    mat4_rotate(mat, NULL, M_PI, Y);
    vec3_add(topQuad->vertices[0].position, bottomQuad->vertices[1].position, up);
    vec3_add(topQuad->vertices[1].position, bottomQuad->vertices[0].position, up);
    vec3_add(topQuad->vertices[2].position, bottomQuad->vertices[3].position, up);
    vec3_add(topQuad->vertices[3].position, bottomQuad->vertices[2].position, up);
    topQuad->orientation = TOP;
    mesh_add_quad(mesh, topQuad);

    mesh_calc_normals(mesh);
    mesh_buffer(mesh, MESH_LINE);

    return mesh;
}

void box_print(Box* box) {
    printf("Position: <%f, %f, %f>\n", box->position[0], box->position[1], box->position[2]);
    printf("Dimensions: %f W x %f H x %f L\n", box->width, box->height, box->length);
    printf("-----\n");
}
