#include "window.h"

Window* windowReference;

void resize(GLFWwindow* glfwWindow, int width, int height) {
    window_resize(windowReference, width, height);
}

Window* window_init(Window* w, Application* application) {
  Window* window = w ? w : NEW(Window, 1);
  window->width = 0;
  window->height = 0;
  
  window->application = application;
  application->window = window;

  return window;
}

void window_open(Window* window) {
    if (!glfwInit())
        return;

    windowReference = window;

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    window->width = mode->width;
    window->height = mode->height;
    
    window->glfwWindow = glfwCreateWindow(mode->width, mode->height, "Voxel", glfwGetPrimaryMonitor(), NULL);
    if (!window->glfwWindow)
        glfwTerminate();

    glfwSetWindowSizeCallback(window->glfwWindow, resize);

    glfwMakeContextCurrent(window->glfwWindow);

    if (window->application->setup)
        window->application->setup(window->application);

    window->application->main(window->application);

    if (window->application->teardown)
        window->application->teardown(window->application);
}

void window_destroy(Window* window) {
    glfwDestroyWindow(window->glfwWindow);
    glfwTerminate();
}

void window_resize(Window* window, int width, int height) {
    window->width = width;
    window->height = height;

    window->application->resize(window->application);
}

