#ifndef MOUSE_H
#define MOUSE_H

#define BUTTON_LEFT   0b10
#define BUTTON_RIGHT  0b01
#define MOUSE_PRESS   0b01
#define MOUSE_RELEASE 0b00

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "global.h"
#include "window.h"

typedef struct {
    Window* window;
} Mouse;

Mouse* mouse_init(Mouse* m, Window* window);

char mouse_state(Mouse* mouse, int* x, int* y);

#endif // MOUSE_H
