#ifndef WINDOW_INTERNAL_H
#define WINDOW_INTERNAL_H

#include "../window.h"

// Static callback
void resize(GLFWwindow* glfwWindow, int width, int height);

// Invoked by static callback
void window_resize(Window* window, int width, int height);

void window_set_icon(Window* window);

#endif // WINDOW_INTERNAL_H
