#ifndef MATRIX_H
#define MATRIX_H

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "global.h"

float* mat4_identity(float* mat4d);
float* mat4_translate(float* mat4d, float* mat4s, float* vec3);
float* mat4_rotate(float* mat4d, float* mat4s, float radians, float* vec3);
float* mat4_multiply(float* mat4d, float* mat4a, float* mat4b);
float* mat4_perspective(float* mat4d, double fov, double aspect, double near, double far);
float* mat4_orthographic(float* mat4d, double left, double right, double top, double bottom);

float mat2_determinate(float* mat2s);

float* mat3_sub(float* mat2d, float* mat3s, unsigned int i);
float* mat3_transpose(float* mat3d, float* mat3s);
float* mat3_inverse(float* mat3d, float* mat3s);
float mat3_determinate(float* mat3s);

float* mat4_sub(float* mat3d, float* mat4s, unsigned int i);
float* mat4_transpose(float* mat4d, float* mat4s);
float* mat4_inverse(float* mat4d, float* mat4s);
float mat4_determinate(float* mat4s);

float* vec3_add(float* vec3d, float* vec3a, float* vec3b);
float* vec3_scale(float* vec3d, float* vec3s, float m);
float* vec3_transform(float* vec3d, float* mat4, float* vec3s);
float* vec3_normalize(float* vec3d, float* vec3s);

float* vec4_transform(float* vec4d, float* mat4, float* vec4s);

#endif // MATRIX_H
