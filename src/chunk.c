#include "chunk.h"
#include "internal/chunk.h"

/* Linked list processing callbacks */

void destroy_mesh(void* ptr) {
    Mesh* mesh = (Mesh*)ptr;
    mesh_destroy(mesh);
    free(mesh);
}

char meshes_are_equal(void* ptrA, void* ptrB) {
    Mesh* meshA = (Mesh*)ptrA;
    Mesh* meshB = (Mesh*)ptrB;

    return meshA->color == meshB->color;
}

void prepare_mesh(void* ptr, void* rendererPtr) {
    Mesh* mesh = (Mesh*)ptr;

    mesh_calc_normals(mesh);
    mesh_buffer(mesh, MESH_FILL);
}

/* Chunk */

Chunk* chunk_init(Chunk* c, Vulkan* vulkan, int width, int height, int length) {
    Chunk* chunk = c ? c : NEW(Chunk, 1);

    chunk->vulkan = vulkan;

    chunk->width = width;
    chunk->height = height;
    chunk->length = length;

    chunk->dirty = 0;

    linked_list_init(&chunk->meshes);

    chunk->blocks = NEW(Block**, width);

    for (int x=0; x<width; x++) {
        chunk->blocks[x] = NEW(Block*, height);

        for (int y=0; y<height; y++) {
            chunk->blocks[x][y] = NEW(Block, length);

            for (int z=0; z<length; z++) {
                chunk->blocks[x][y][z].data = 0;
            }
        }
    }

    return chunk;
}

void chunk_destroy(Chunk* chunk) {
    for (int x=0; x<chunk->width; x++) {
        for (int y=0; y<chunk->height; y++) {
            free(chunk->blocks[x][y]);
        }
        free(chunk->blocks[x]);
    }
    free(chunk->blocks);

    linked_list_destroy(&chunk->meshes, destroy_mesh);
}

void chunk_mesh(Chunk* chunk) {
    linked_list_destroy(&chunk->meshes, destroy_mesh);
    linked_list_init(&chunk->meshes);

    int b, d, i, j, k, l, w, h, u, v, n;

    int  x[3] = {0, 0, 0};
    int  q[3] = {0, 0, 0};
    int du[3] = {0, 0, 0};
    int dv[3] = {0, 0, 0};
    int lim[3] = {
        chunk->width,
        chunk->height,
        chunk->length
    };

    uint16_t* mask;

    uint16_t face;
    uint16_t face1;

    int side;

    for (b=0; b<2; b++) {

        for (d=0; d<3; d++) {

            u = (d+1) % 3;
            v = (d+2) % 3;

            x[0] = 0;
            x[1] = 0;
            x[2] = 0;

            q[0] = 0;
            q[1] = 0;
            q[2] = 0;
            q[d] = 1;

            for (x[d] = -1; x[d] < lim[d];) {

                mask = NEW(uint16_t, lim[u] * lim[v]);

                n = 0;

                for (x[u] = 0; x[u] < lim[u]; x[u]++) {

                    for (x[v] = 0; x[v] < lim[v]; x[v]++) {
                        face  = (x[d] >= 0)
                            ? block_is_active(&chunk->blocks[x[0]][x[1]][x[2]])
                                ? block_color(&chunk->blocks[x[0]][x[1]][x[2]])
                                : -1
                            : -1;
                        face1 = (x[d] < (lim[d] - 1))
                            ? block_is_active(&chunk->blocks[x[0]+q[0]][x[1]+q[1]][x[2]+q[2]])
                                ? block_color(&chunk->blocks[x[0]+q[0]][x[1]+q[1]][x[2]+q[2]])
                                : -1
                            : -1;

                        if (face == face1 || (!b && face != (uint16_t)-1) || (b && face1 != (uint16_t)-1)) {
                            mask[n++] = -1;
                        } else {
                            mask[n++] = b ? face : face1;
                        }
                    }
                }

                x[d]++;

                if (d == 0) {
                    side = b ? EAST  : WEST;
                }
                else if (d == 1) {
                    side = b ? TOP   : BOTTOM;
                }
                else if (d == 2) {
                    side = b ? SOUTH : NORTH;
                }

                n = 0;

                for (j = 0; j < lim[u]; j++) {

                    for (i = 0; i < lim[v];) {

                        if (mask[n] != (uint16_t)-1) {

                            for (w = 1; i+w < lim[v] && mask[n+w] == mask[n]; w++) {}

                            char done = 0;
                            for (h = 1; j+h < lim[u]; h++) {

                                for (k = 0; k < w; k++) {

                                    if (mask[n+k+h*lim[v]] != mask[n]) {
                                        done = 1;
                                        break;
                                    }
                                }

                                if (done) break;
                            }

                            x[u] = j;
                            x[v] = i;

                            du[0] = 0;
                            du[1] = 0;
                            du[2] = 0;
                            du[u] = h;

                            dv[0] = 0;
                            dv[1] = 0;
                            dv[2] = 0;
                            dv[v] = w;

                            Quad* quad = NEW(Quad, 1);

                            for (k=0; k<4; k++) {
                                for (l=0; l<3; l++) {
                                    if (b) {
                                        switch (k) {
                                            case 0:
                                                quad->vertices[k].position[l] = (float)x[l]+du[l]+dv[l];
                                                break;
                                            case 1:
                                                quad->vertices[k].position[l] = (float)x[l]+dv[l];
                                                break;
                                            case 2:
                                                quad->vertices[k].position[l] = (float)x[l]+du[l];
                                                break;
                                            case 3:
                                                quad->vertices[k].position[l] = (float)x[l];
                                            default:
                                                break;
                                        }
                                    } else {
                                        switch (k) {
                                            case 0:
                                                quad->vertices[k].position[l] = (float)x[l]+du[l];
                                                break;
                                            case 1:
                                                quad->vertices[k].position[l] = (float)x[l];
                                                break;
                                            case 2:
                                                quad->vertices[k].position[l] = (float)x[l]+du[l]+dv[l];
                                                break;
                                            case 3:
                                                quad->vertices[k].position[l] = (float)x[l]+dv[l];
                                            default:
                                                break;
                                        }
                                    }
                                }
                            }

                            quad->orientation = side;

                            Mesh* mesh = NEW(Mesh, 1);
                            mesh->color = mask[n];
                            LinkedListNode* existingNode = linked_list_find(&chunk->meshes, mesh, meshes_are_equal);
                            if (existingNode == NULL) {
                                mesh_init(mesh, chunk->vulkan);
                                linked_list_insert(&chunk->meshes, mesh);
                            } else {
                                free(mesh);
                                mesh = (Mesh*)existingNode->data;
                            }
                            mesh_add_quad(mesh, quad);

                            for (l = 0; l < h; l++) {
                                for (k = 0; k < w; k++) {
                                    mask[n+k+l*lim[v]] = -1;
                                }
                            }

                            i += w;
                            n += w;

                        } else {

                            i++;
                            n++;

                        }
                    }
                }

                free(mask);
            }
        }
    }

    linked_list_foreach(&chunk->meshes, prepare_mesh, NULL);
}

