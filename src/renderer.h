#ifndef RENDERER_H
#define RENDERER_H

#include "shader.h"
#include "matrix.h"

typedef struct {
    ShaderProgram3D shaderProgram3D;
    ShaderProgram2D shaderProgram2D;
} Renderer;

Renderer* renderer_init(Renderer* r);
void renderer_destroy(Renderer* renderer);

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

#endif // RENDERER_H
