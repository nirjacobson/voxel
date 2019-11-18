#ifndef GLOBAL_H
#define GLOBAL_H

#include "GLES2/gl2.h"

#define MIN(x, y)    ((x) < (y) ? (x) : (y))
#define MAX(x, y)    ((x) > (y) ? (x) : (y))
#define NEW(type, count)     (type*)malloc(count * sizeof(type));

#define glCheck() assert(glGetError() == 0)

float X[3];
float Y[3];
float Z[3];

#endif // GLOBAL_H
