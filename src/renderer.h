#ifndef RENDERER_H
#define RENDERER_H

#include <math.h>

#include "shader.h"
#include "matrix.h"

#include "camera.h"
#include "ground.h"
#include "chunk.h"
#include "mesh.h"
#include "world.h"
#include "picker.h"
#include "panel.h"

typedef struct {
    ShaderProgram3D shaderProgram3D;
    ShaderProgram2D shaderProgram2D;
} Renderer;

Renderer* renderer_init(Renderer* r);
void renderer_destroy(Renderer* renderer);

void renderer_clear(Renderer* renderer);

void renderer_3D_update_model(Renderer* renderer, float* mat4);
void renderer_3D_update_camera(Renderer* renderer, float* mat4);
void renderer_3D_update_projection(Renderer* renderer, float* mat4);

void renderer_3D_update_world_position(Renderer* renderer, float* position);

void renderer_3D_update_color(Renderer* renderer, float r, float g, float b);
void renderer_3D_update_ambient(Renderer* renderer, float a);
void renderer_3D_update_sun_position(Renderer* renderer, float* position);

void renderer_3D_apply_camera(Renderer* renderer, Camera* camera);

void renderer_3D_use(Renderer* renderer);

void renderer_2D_update_projection(Renderer* renderer, float* mat4);
void renderer_2D_update_sampler(Renderer* renderer, GLint sampler);

void renderer_2D_use(Renderer* renderer);

void renderer_render_ground(Renderer* renderer, Ground* ground, Camera* camera);
void renderer_render_chunk(Renderer* renderer, Chunk* chunk, float* position);
void renderer_render_mesh(Renderer* renderer, Mesh* mesh, char mode);
void renderer_render_world(Renderer* renderer, World* world, Camera* camera);
void renderer_render_picker(Renderer* renderer, Picker* picker);
void renderer_render_panels(Renderer* renderer, LinkedList* panels);

#endif // RENDERER_H
