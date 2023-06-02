#ifndef SHADER_INTERNAL_H
#define SHADER_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>

#include "../shader.h"

GLuint shader_create(const char* path, GLenum shaderType);
GLuint shader_create_program(GLuint vertex_shader, GLuint fragment_shader);

#endif // SHADER_INTERNAL_H
