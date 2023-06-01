#ifndef CHUNK_INTERNAL_H
#define CHUNK_INTERNAL_H

#include "../chunk.h"

/* Linked list processing callbacks */

void destroy_mesh(void* ptr);
char meshes_are_equal(void* ptrA, void* ptrB);
void prepare_mesh(void* node, void* rendererPtr);
void draw_mesh(void* node, void* renderer);

#endif // CHUNK_INTERNAL_H