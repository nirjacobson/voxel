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

cairo_status_t cairo_read(void* closure, unsigned char* data, unsigned int length) {
    unsigned int read_len = g_input_stream_read((GInputStream*)closure, data, length, (void*)NULL, (GError**)NULL);

    if (read_len == length) {
        return CAIRO_STATUS_SUCCESS;
    }

    return CAIRO_STATUS_READ_ERROR;
}

void window_set_icon(Window* window) {
    GInputStream* inputStream = g_resources_open_stream("/res/voxel.png", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
    cairo_surface_t* surface  = cairo_image_surface_create_from_png_stream(cairo_read, (void*)inputStream);
    unsigned char* pixels = cairo_image_surface_get_data(surface);
    GLFWimage image = {
        .width = cairo_image_surface_get_width(surface),
        .height = cairo_image_surface_get_height(surface),
        .pixels = pixels
    };

    for (int i = 0; i < image.width * image.height * 4; i+=4) {
        unsigned char temp = pixels[i];
        pixels[i] = pixels[i + 2];
        pixels[i + 2] = temp;
    }

    glfwSetWindowIcon(window->glfwWindow, 1, &image);

    cairo_surface_destroy(surface);
}

void window_open(Window* window) {
    if (!glfwInit()) {
        return;
    }

    window->width = WINDOW_DEFAULT_WIDTH;
    window->height = WINDOW_DEFAULT_HEIGHT;

    bool vulkan = glfwVulkanSupported() && !getenv("FORCE_OPENGL");

    if (vulkan) {
        Vulkan vulk;
        vulkan = vulkan && vulkan_create_instance("Voxel", &vulk.instance);

        if (vulkan) {
            PFN_vkDestroyInstance pfnDestroyInstance =
                (PFN_vkDestroyInstance)glfwGetInstanceProcAddress(vulk.instance, "vkDestroyInstance");
            pfnDestroyInstance(vulk.instance, NULL);

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }
    }

    window->glfwWindow = glfwCreateWindow(window->width, window->height, "Voxel", NULL, NULL);
    if (!window->glfwWindow) {
        glfwTerminate();
    }

    window_set_icon(window);

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
