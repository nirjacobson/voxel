#include "keyboard.h"

Keyboard* keyboard_init(Keyboard* k, Window* window) {
    Keyboard* keyboard = k ? k : NEW(Keyboard, 1);

    keyboard->window = window;

    return keyboard;
}

char keyboard_key_is_pressed(Keyboard* keyboard, int key) {
    return glfwGetKey(keyboard->window->glfwWindow, key) == GLFW_PRESS;
}
