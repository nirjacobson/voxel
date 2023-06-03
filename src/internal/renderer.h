#ifndef RENDERER_INTERNAL_H
#define RENDERER_INTERNAL_H

#include "../renderer.h"

#include "../ground.h"
#include "../chunk.h"
#include "../mesh.h"
#include "../panel.h"

void renderer_3D_update_model(Renderer* renderer, float* mat4);
void renderer_3D_update_camera(Renderer* renderer, float* mat4);
void renderer_3D_update_projection(Renderer* renderer, float* mat4);

void renderer_3D_update_world_position(Renderer* renderer, float* position);

void renderer_3D_update_color(Renderer* renderer, float r, float g, float b);
void renderer_3D_update_ambient(Renderer* renderer, float a);
void renderer_3D_update_sun_position(Renderer* renderer, float* position);

void renderer_3D_use(Renderer* renderer);

void renderer_2D_update_projection(Renderer* renderer, float* mat4);
void renderer_2D_update_sampler(Renderer* renderer, GLint sampler);

void renderer_2D_use(Renderer* renderer);

void render_mesh(void* ptr, void* rendererPtr);
void render_world_chunk(void* worldChunkPtr, void* rendererPtr);
void render_panel(void* panelPtr, void* rendererPtr);

void renderer_render_ground(Renderer* renderer, Ground* ground, Camera* camera);
void renderer_render_chunk(Renderer* renderer, Chunk* chunk, float* position);
void renderer_render_mesh(Renderer* renderer, Mesh* mesh, char mode);
void renderer_render_panel(Renderer* renderer, Panel* panel);

#endif // RENDERER_INTERNAL_H
