#ifndef WINDOW_H
#define WINDOW_H

#define WINDOW_DEFAULT_WIDTH  1440
#define WINDOW_DEFAULT_HEIGHT 900

#define GLFW_INCLUDE_ES3

#include <assert.h>

#include "global.h"
#include "application.h"

struct Window;
struct Voxel;

struct Window {
  GLFWwindow* glfwWindow;
  int width;
  int height;

  Application* application;
};

typedef struct Window Window;

Window* window_init(Window* w, Application* application);
void window_open(Window* window);
void window_destroy(Window* window);
void window_resize(Window* window, int width, int height);
void window_toggle_fullscreen(Window* window);

#endif // WINDOW_H
