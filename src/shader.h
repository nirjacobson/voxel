#ifndef SHADER_H
#define SHADER_H

#include <stdio.h>

#include <GLES3/gl3.h>

#include "global.h"

/* ShaderProgram3D */

typedef struct {
    GLuint shader_vert;
    GLuint shader_frag;
    GLuint shader_prog;

    GLint attrib_position;
    GLint attrib_normal;

    GLint unifrm_world_position;

    GLint unifrm_model;
    GLint unifrm_camera;
    GLint unifrm_projection;

    GLint unifrm_color;
    GLint unifrm_ambient;
    GLint unifrm_sun_position;
} ShaderProgram3D;

ShaderProgram3D* shader_program_3D_init(ShaderProgram3D* s);
void shader_program_3D_destroy(ShaderProgram3D* shaderProgram3D);

void shader_program_3D_update_world_position(ShaderProgram3D* shaderProgram3D, float* position);

void shader_program_3D_update_model(ShaderProgram3D* shaderProgram3D, float* mat4);
void shader_program_3D_update_camera(ShaderProgram3D* shaderProgram3D, float* mat4);
void shader_program_3D_update_projection(ShaderProgram3D* shaderProgram3D, float* mat4);

void shader_program_3D_update_color(ShaderProgram3D* shaderProgram3D, float r, float g, float b);
void shader_program_3D_update_ambient(ShaderProgram3D* shaderProgram3D, float a);
void shader_program_3D_update_sun_position(ShaderProgram3D* shaderProgram3D, float* position);

void shader_program_3D_use(ShaderProgram3D* shaderProgram3D);

/* ShaderProgram2D */

typedef struct {
    GLuint shader_vert;
    GLuint shader_frag;
    GLuint shader_prog;

    GLint attrib_position;
    GLint attrib_texcoord;

    GLint unifrm_projection;

    GLint unifrm_sampler;
} ShaderProgram2D;

ShaderProgram2D* shader_program_2D_init(ShaderProgram2D* s);
void shader_program_2D_destroy(ShaderProgram2D* shaderProgram2D);

void shader_program_2D_update_projection(ShaderProgram2D* shaderProgram2D, float* mat4);
void shader_program_2D_update_sampler(ShaderProgram2D* shaderProgram2D, GLint sampler);

void shader_program_2D_use(ShaderProgram2D* shaderProgram2D);

#endif // SHADER_H
