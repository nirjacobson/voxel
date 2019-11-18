#include "renderer.h"

Renderer* renderer_init(Renderer* r) {
    Renderer* renderer = r ? r : NEW(Renderer, 1);

    shader_program_3D_init(&renderer->shaderProgram3D);

    float mat4[16];
    renderer_3D_update_model(renderer, mat4_identity(mat4));
    renderer_3D_update_ambient(renderer, 0.4);

    float sunPosition[] = { 100, 100, 100 };
    renderer_3D_update_sun_position(renderer, sunPosition);

    shader_program_2D_init(&renderer->shaderProgram2D);

    glActiveTexture(GL_TEXTURE0);
    renderer_2D_update_sampler(renderer, 0);


    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearDepthf(1);
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glLineWidth(2);
    
    return renderer;
}

void renderer_destroy(Renderer* renderer) {
    shader_program_2D_destroy(&renderer->shaderProgram2D);
    shader_program_3D_destroy(&renderer->shaderProgram3D);
}

void renderer_3D_update_world_position(Renderer* renderer, float* position) {
  shader_program_3D_use(&renderer->shaderProgram3D);
  shader_program_3D_update_world_position(&renderer->shaderProgram3D, position);
}

void renderer_3D_update_model(Renderer* renderer, float* mat4) {
  shader_program_3D_use(&renderer->shaderProgram3D);
  shader_program_3D_update_model(&renderer->shaderProgram3D, mat4);
}

void renderer_3D_update_camera(Renderer* renderer, float* mat4) {
  shader_program_3D_use(&renderer->shaderProgram3D);
  shader_program_3D_update_camera(&renderer->shaderProgram3D, mat4);
}

void renderer_3D_update_projection(Renderer* renderer, float* mat4) {
  shader_program_3D_use(&renderer->shaderProgram3D);
  shader_program_3D_update_projection(&renderer->shaderProgram3D, mat4);
}

void renderer_3D_update_color(Renderer* renderer, float r, float g, float b) {
  shader_program_3D_use(&renderer->shaderProgram3D);
  shader_program_3D_update_color(&renderer->shaderProgram3D, r, g, b);
}

void renderer_3D_update_ambient(Renderer* renderer, float a) {
  shader_program_3D_use(&renderer->shaderProgram3D);
  shader_program_3D_update_ambient(&renderer->shaderProgram3D, a);
}

void renderer_3D_update_sun_position(Renderer* renderer, float* position) {
  shader_program_3D_use(&renderer->shaderProgram3D);
  shader_program_3D_update_sun_position(&renderer->shaderProgram3D, position);
}

void renderer_3D_use(Renderer* renderer) {
    shader_program_3D_use(&renderer->shaderProgram3D);
}

void renderer_2D_update_projection(Renderer* renderer, float* mat4) {
  shader_program_2D_use(&renderer->shaderProgram2D);
  shader_program_2D_update_projection(&renderer->shaderProgram2D, mat4);
}

void renderer_2D_update_sampler(Renderer* renderer, GLint sampler) {
  shader_program_2D_use(&renderer->shaderProgram2D);
  shader_program_2D_update_sampler(&renderer->shaderProgram2D, sampler);
}

void renderer_2D_use(Renderer* renderer) {
    shader_program_2D_use(&renderer->shaderProgram2D);
}
