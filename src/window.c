#include "window.h"
#include "internal/window.h"
#include "voxel.h"

void resize(GLFWwindow* glfwWindow, int width, int height) {
    window_resize((Window*)glfwGetWindowUserPointer(glfwWindow), width, height);
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
    if (!glfwInit()) {
        return;
    }

    window->width = WINDOW_DEFAULT_WIDTH;
    window->height = WINDOW_DEFAULT_HEIGHT;

    bool vulkan = glfwVulkanSupported() && !getenv("FORCE_OPENGL");
    if (vulkan) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    window->glfwWindow = glfwCreateWindow(window->width, window->height, "Voxel", NULL, NULL);
    if (!window->glfwWindow) {
        glfwTerminate();
    }

    glfwSetWindowUserPointer(window->glfwWindow, window);

    glfwSetWindowSizeCallback(window->glfwWindow, resize);
    glfwMakeContextCurrent(window->glfwWindow);
    glfwSetFramebufferSizeCallback(window->glfwWindow, resize);

    if (!vulkan) {
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            printf("GLEW::Error : failed to initialize GLEW\n");
        }
    }

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
        window_resize(window, WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);
    }
}

char window_key_is_pressed(Window* window, int key) {
    return glfwGetKey(window->glfwWindow, key) == GLFW_PRESS;
}

char window_mouse_state(Window* window, int* x, int* y) {
    double xd, yd;
    glfwGetCursorPos(window->glfwWindow, &xd, &yd);
    *x = (int)xd;
    *y = (int)yd;

    char buttons = glfwGetMouseButton(window->glfwWindow, GLFW_MOUSE_BUTTON_LEFT);
    buttons <<= 1;
    buttons |= glfwGetMouseButton(window->glfwWindow, GLFW_MOUSE_BUTTON_RIGHT);

    return buttons;
}
