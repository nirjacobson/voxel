#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "global.h"
#include "window.h"

typedef struct {
    Window* window;
} Keyboard;

Keyboard* keyboard_init(Keyboard* k, Window* window);
char keyboard_key_is_pressed(Keyboard* keyboard, int key);

#endif // KEYBOARD_H
