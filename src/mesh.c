#include "mesh.h"

/* Helpers */

void mesh_ortn_to_normal(char orientation, float* nvec) {
    switch (orientation) {
        case NORTH:
            nvec[0] =  0;
            nvec[1] =  0;
            nvec[2] = -1;
            break;
        case SOUTH:
            nvec[0] =  0;
            nvec[1] =  0;
            nvec[2] =  1;
            break;
        case WEST:
            nvec[0] = -1;
            nvec[1] =  0;
            nvec[2] =  0;
            break;
        case EAST:
            nvec[0] =  1;
            nvec[1] =  0;
            nvec[2] =  0;
            break;
        case TOP:
            nvec[0] =  0;
            nvec[1] =  1;
            nvec[2] =  0;
            break;
        case BOTTOM:
            nvec[0] =  0;
            nvec[1] = -1;
            nvec[2] =  0;
        default:
            break;
    }
}

void mesh_add_quad(Mesh* mesh, Quad* quad) {
    linked_list_insert(&mesh->quads, quad);
}

void quad_set_normals(void* ptr, void* unused) {
    Quad* quad = (Quad*)ptr;

    float normal[3];
    mesh_ortn_to_normal(quad->orientation, normal);
    for (int v = 0; v < 4; v++) {
        for (int i = 0; i < 3; i++) {
            quad->vertices[v].normal_v[i] = normal[i];
        }
    }
}

/* Mesh */

Mesh* mesh_init(Mesh* m) {
    Mesh* mesh = m ? m : NEW(Mesh, 1);

    linked_list_init(&mesh->quads);

    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    return mesh;
}

void mesh_destroy(Mesh* mesh) {
    glDeleteBuffers(1, &mesh->ebo);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteVertexArrays(1, &mesh->vao);

    linked_list_destroy(&mesh->quads, free);
}

void mesh_calc_normals(Mesh* mesh) {
    linked_list_foreach(&mesh->quads, quad_set_normals, NULL);
}

void mesh_buffer(Mesh* mesh, char mode) {
    int num_elements_f = mesh->quads.size * 4;
    int num_vertices_f = num_elements_f * 6;
    GLfloat* vertex_data = NEW(GLfloat, num_vertices_f);
    GLushort*  elements    = NEW(GLushort, num_elements_f);

    LinkedListNode* node = mesh->quads.head;
    for (int q=0; node; q++, node = node->next) {
        Quad* quad = (Quad*)node->data;
        for (int v=0; v<4; v++) {
            for (int p=0; p<3; p++)
                vertex_data[q*24+v*6+0+p] = quad->vertices[v].position[p];
            for (int n=0; n<3; n++)
                vertex_data[q*24+v*6+3+n] = quad->vertices[v].normal_v[n];

            int order[] = {
                quad->vt_order[0],
                quad->vt_order[1],
                mode == MESH_FILL ? quad->vt_order[2] : quad->vt_order[3],
                mode == MESH_FILL ? quad->vt_order[3] : quad->vt_order[2],
            };

            elements[q*4+v] = q*4 + order[v];
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, num_vertices_f*sizeof(GLfloat), vertex_data, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_elements_f*sizeof(GLushort), elements, GL_STATIC_DRAW);

    free(elements);
    free(vertex_data);
}

void mesh_draw(Mesh* mesh, Renderer* renderer, char mode) {
    glBindVertexArray(mesh->vao);
    renderer_3D_use(renderer);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);

    glEnableVertexAttribArray(renderer->shaderProgram3D.attrib_position);
    glVertexAttribPointer(renderer->shaderProgram3D.attrib_position, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(renderer->shaderProgram3D.attrib_normal);
    glVertexAttribPointer(renderer->shaderProgram3D.attrib_normal, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));

    for (int q=0; q<mesh->quads.size; q++)
        glDrawElements(mode == MESH_FILL ? GL_TRIANGLE_STRIP : GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*) (4*q*sizeof(GLushort)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
