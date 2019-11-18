#ifndef MOUSE_H
#define MOUSE_H

#define BUTTON_LEFT   0b01
#define BUTTON_RIGHT  0b10
#define MOUSE_PRESS   0b01
#define MOUSE_RELEASE 0b00

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/input.h>

#include "global.h"

typedef struct {
    int fd;
    char buttons;
    int x;
    int y;

    struct {
        int left;
        int right;
        int top;
        int bottom;
    } bounds;
} Mouse;

Mouse* mouse_init(Mouse* m, const char* devicePath);
void mouse_destroy(Mouse* mouse);

void mouse_set_bounds(Mouse* mouse, int left, int right, int top, int bottom);
void mouse_process_events(Mouse* mouse);
char mouse_state(Mouse* mouse, int* x, int* y);

#endif // MOUSE_H
