#ifndef RENDERER_INTERNAL_H
#define RENDERER_INTERNAL_H

#include <GLES3/gl3.h>

#include "../renderer.h"

#include "../ground.h"
#include "../chunk.h"
#include "../mesh.h"

void renderer_3D_update_model(Renderer* renderer, float* mat4);
void renderer_3D_update_camera(Renderer* renderer, float* mat4);
void renderer_3D_update_projection(Renderer* renderer, float* mat4);

void renderer_3D_update_world_position(Renderer* renderer, float* position);

void renderer_3D_update_color(Renderer* renderer, float r, float g, float b);
void renderer_3D_update_ambient(Renderer* renderer, float a);
void renderer_3D_update_sun_position(Renderer* renderer, float* position);

void renderer_3D_use(Renderer* renderer);

void render_mesh(void* ptr, void* rendererPtr);
void render_world_chunk(void* worldChunkPtr, void* rendererPtr);

void renderer_render_ground(Renderer* renderer, Ground* ground, Camera* camera);
void renderer_render_mesh(Renderer* renderer, Mesh* mesh, char mode);
void renderer_render_chunk(Renderer* renderer, Chunk* chunk, float* position);

#endif // RENDERER_INTERNAL_H
