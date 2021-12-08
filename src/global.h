#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdlib.h>

#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>

#define MIN(x, y)    ((x) < (y) ? (x) : (y))
#define MAX(x, y)    ((x) > (y) ? (x) : (y))
#define NEW(type, count)     (type*)malloc(count * sizeof(type));

#define glCheck() assert(glGetError() == 0)

extern float X[3];
extern float Y[3];
extern float Z[3];

#endif // GLOBAL_H
