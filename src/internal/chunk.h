#ifndef CHUNK_INTERNAL_H
#define CHUNK_INTERNAL_H

#include "../chunk.h"

typedef struct {
    uint16_t color;
    Mesh mesh;
} MeshListNode;

/* Linked list processing callbacks */

void mesh_list_node_mesh_destroy(void* node);
char mesh_list_nodes_equal(void* nodeA, void* nodeB) ;
void mesh_list_node_prepare(void* node, void* rendererPtr);
void mesh_list_node_draw(void* node, void* renderer);

#endif // CHUNK_INTERNAL_H