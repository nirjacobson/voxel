#include "window.h"
#include "internal/window.h"

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

    window->width = WINDOW_DEFAULT_WIDTH;
    window->height = WINDOW_DEFAULT_HEIGHT;

    window->glfwWindow = glfwCreateWindow(window->width, window->height, "Voxel", NULL, NULL);
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

void window_toggle_fullscreen(Window* window) {
    if (glfwGetWindowMonitor(window->glfwWindow) == NULL) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window->glfwWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        glfwSetWindowMonitor(window->glfwWindow, NULL, 0, 0, WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT, GLFW_DONT_CARE);
    }
}
