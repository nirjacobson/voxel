#ifndef WINDOW_H
#define WINDOW_H

#define WINDOW_DEFAULT_WIDTH  1440
#define WINDOW_DEFAULT_HEIGHT 900

#define MOUSE_BUTTON_LEFT  0b10
#define MOUSE_BUTTON_RIGHT 0b01
#define MOUSE_PRESS        0b01
#define MOUSE_RELEASE      0b00

#define GLFW_INCLUDE_ES3
#define GLFW_INCLUDE_VULKAN

#include <assert.h>

#include <GLFW/glfw3.h>

#include "global.h"
#include "application.h"

struct Window;
struct Voxel;

struct Window {
    GLFWwindow* glfwWindow;
    int width;
    int height;

    Application* application;

    VkSurfaceKHR surface;
};

typedef struct Window Window;

Window* window_init(Window* w, Application* application);
void window_open(Window* window);
void window_destroy(Window* window);
void window_toggle_fullscreen(Window* window);
char window_key_is_pressed(Window* window, int key);
char window_mouse_state(Window* window, int* x, int* y);

#endif // WINDOW_H
