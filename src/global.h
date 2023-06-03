#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdlib.h>

#define NEW(type, count)     (type*)malloc((count) * sizeof(type));

extern float X[3];
extern float Y[3];
extern float Z[3];

#endif // GLOBAL_H
