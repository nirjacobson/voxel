#include "mouse.h"

Mouse* mouse_init(Mouse* m, Window* window) {
    Mouse* mouse = m ? m : NEW(Mouse, 1);

    memset(mouse, 0, sizeof(Mouse));

    mouse->window = window;

    mouse->bounds.left = -1;
    mouse->bounds.right = -1;
    mouse->bounds.top = -1;
    mouse->bounds.bottom = -1;

    return mouse;
}

void mouse_set_bounds(Mouse* mouse) {
    mouse->bounds.left = 0;
    mouse->bounds.right = mouse->window->width;
    mouse->bounds.top = 0;
    mouse->bounds.bottom = mouse->window->height;
}

char mouse_state(Mouse* mouse, int* x, int* y) {
    double xd, yd;
    glfwGetCursorPos(mouse->window->glfwWindow, &xd, &yd);
    *x = (int)xd;
    *y = (int)yd;
    
    mouse->buttons = glfwGetMouseButton(mouse->window->glfwWindow, GLFW_MOUSE_BUTTON_LEFT);
    mouse->buttons <<= 1;
    mouse->buttons |= glfwGetMouseButton(mouse->window->glfwWindow, GLFW_MOUSE_BUTTON_RIGHT);
    
    return mouse->buttons;
}

