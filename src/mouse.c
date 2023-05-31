#include "mouse.h"

Mouse* mouse_init(Mouse* m, Window* window) {
    Mouse* mouse = m ? m : NEW(Mouse, 1);

    memset(mouse, 0, sizeof(Mouse));

    mouse->window = window;

    return mouse;
}

char mouse_state(Mouse* mouse, int* x, int* y) {
    double xd, yd;
    glfwGetCursorPos(mouse->window->glfwWindow, &xd, &yd);
    *x = (int)xd;
    *y = (int)yd;

    char buttons = glfwGetMouseButton(mouse->window->glfwWindow, GLFW_MOUSE_BUTTON_LEFT);
    buttons <<= 1;
    buttons |= glfwGetMouseButton(mouse->window->glfwWindow, GLFW_MOUSE_BUTTON_RIGHT);

    return buttons;
}

