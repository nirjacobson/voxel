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
void renderer_resize(Renderer* renderer, int width, int height, Camera* camera);
void renderer_apply_camera(Renderer* renderer, Camera* camera);
void renderer_render_world(Renderer* renderer, World* world, Camera* camera);
void renderer_render_picker(Renderer* renderer, Picker* picker);
void renderer_render_panels(Renderer* renderer, LinkedList* panels);

#endif // RENDERER_H
