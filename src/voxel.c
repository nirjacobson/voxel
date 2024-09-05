#include "voxel.h"
#include "internal/voxel.h"

#ifdef _WIN32
#include <Windows.h>
int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#define	timersub(a, b, result)						    \
  do {									                \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;		\
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;	\
    if ((result)->tv_usec < 0) {					    \
      --(result)->tv_sec;						        \
      (result)->tv_usec += 1000000;					    \
    }									                \
  } while (0)

#endif

extern const bool enableValidationLayers;

extern GResource* resources_get_resource();

Voxel* voxel_init(Voxel* v) {
    Voxel* voxel = v ? v : NEW(Voxel, 1);

    application_init(&voxel->application, voxel, voxel_setup, voxel_main, voxel_resize, voxel_teardown);

    window_init(&voxel->window, &voxel->application);

    return voxel;
}

void voxel_destroy(Voxel* voxel) {
    window_destroy(&voxel->window);
}

char voxel_process_input(Voxel* voxel) {
    static int mouseX[2];
    static int mouseY[2];
    static char mouseButtons[2];
    static char f[2];
    static char z[2];
    static char c[2];

    static int tab = 0;

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_ESCAPE)) {
        return 0;
    }

    mouseX[1] = mouseX[0];
    mouseY[1] = mouseY[0];
    mouseButtons[1] = mouseButtons[0];
    mouseButtons[0] = window_mouse_state(&voxel->window, &mouseX[0], &mouseY[0]);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_W))
        camera_move(&voxel->camera, voxel->camera.forward, 0.5);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_A))
        camera_move(&voxel->camera, voxel->camera.right, -0.5);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_S))
        camera_move(&voxel->camera, voxel->camera.forward, -0.5);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_D))
        camera_move(&voxel->camera, voxel->camera.right, 0.5);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_UP))
        camera_rotate(&voxel->camera, voxel->camera.right, 0.05);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_LEFT))
        camera_rotate(&voxel->camera, Y, 0.05);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_DOWN))
        camera_rotate(&voxel->camera, voxel->camera.right, -0.05);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_RIGHT))
        camera_rotate(&voxel->camera, Y, -0.05);

    renderer_apply_camera(&voxel->renderer, &voxel->camera);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_TAB)) {
        if (!tab) {
            tab = 1;
            voxel->picker.mode = voxel->picker.mode == PICKER_ONTO ? PICKER_ADJACENT : PICKER_ONTO;
            float nx = (2.0 * ((float)mouseX[0])/voxel->window.width) - 1;
            float ny = (2.0 * ((float)(voxel->window.height - mouseY[0]))/voxel->window.height) - 1;
            if (voxel->vulkan) {
                ny = -ny;
            }
            picker_update(&voxel->picker, &voxel->camera, nx, ny);
        }
    } else {
        tab = 0;
    }
    
    z[1] = z[0];
    z[0] = window_key_is_pressed(&voxel->window, GLFW_KEY_Z);

    if (!z[1] && z[0]) {
        if (window_key_is_pressed(&voxel->window, GLFW_KEY_LEFT_CONTROL)) {
            if (window_key_is_pressed(&voxel->window, GLFW_KEY_LEFT_SHIFT)) {
                undo_stack_redo(&voxel->undoStack);
            } else {
                undo_stack_undo(&voxel->undoStack);
            }
        } else {
            picker_set_action(&voxel->picker, PICKER_SELECT);
        }
    }

    if(window_key_is_pressed(&voxel->window, GLFW_KEY_1))
        picker_set_action(&voxel->picker, PICKER_SET);

    if(window_key_is_pressed(&voxel->window, GLFW_KEY_2))
        picker_set_action(&voxel->picker, PICKER_CLEAR);

    if(window_key_is_pressed(&voxel->window, GLFW_KEY_Q))
        picker_set_action(&voxel->picker, PICKER_EYEDROPPER);

    if(window_key_is_pressed(&voxel->window, GLFW_KEY_X))
        picker_set_action(&voxel->picker, PICKER_STAMP);

    c[1] = c[0];
    c[0] = window_key_is_pressed(&voxel->window, GLFW_KEY_C);
    if (!c[1] && c[0]) {
        picker_set_action(&voxel->picker, PICKER_MOVE);
    }

    f[1] = f[0];
    f[0] = window_key_is_pressed(&voxel->window, GLFW_KEY_F);
    if (!f[1] && f[0]) {
        window_toggle_fullscreen(&voxel->window);
    }

    if (mouseX[0] != mouseX[1] || mouseY[0] != mouseY[1]) {
        if (voxel->panelManager.dragging) {
            panel_translate(voxel->panelManager.active_panel, mouseX[0] - mouseX[1], mouseY[0] - mouseY[1]);
        } else {
            Panel* panel = panel_manager_find_panel(&voxel->panelManager, mouseX[0], mouseY[0]);
            if (!panel) {
                float nx = (2.0 * ((float)mouseX[0])/voxel->window.width) - 1;
                float ny = (2.0 * ((float)(voxel->window.height - mouseY[0]))/voxel->window.height) - 1;
                if (voxel->vulkan) {
                    ny = -ny;
                }
                picker_update(&voxel->picker, &voxel->camera, nx, ny);
            }
        }
    }

    if ((mouseButtons[0] & MOUSE_BUTTON_LEFT) != (mouseButtons[1] & MOUSE_BUTTON_LEFT)) {
        if (!(mouseButtons[0] & MOUSE_BUTTON_LEFT)) {
            voxel->panelManager.dragging = 0;
        }

        Panel* panel = panel_manager_find_panel(&voxel->panelManager, mouseX[0], mouseY[0]);

        if (panel) {
            GLuint action = (mouseButtons[0] & MOUSE_BUTTON_LEFT) ? MOUSE_PRESS : MOUSE_RELEASE;
            panel_action(panel, action, mouseX[0], mouseY[0]);
        } else if (mouseButtons[0] & MOUSE_BUTTON_LEFT) {
            char modifier1 = window_key_is_pressed(&voxel->window, GLFW_KEY_LEFT_SHIFT);
            char modifier2 = window_key_is_pressed(&voxel->window, GLFW_KEY_LEFT_SUPER);
            picker_press(&voxel->picker, modifier1, modifier2);
        } else {
            char modifier1 = window_key_is_pressed(&voxel->window, GLFW_KEY_LEFT_SHIFT);
            char modifier2 = window_key_is_pressed(&voxel->window, GLFW_KEY_LEFT_SUPER);
            picker_release(&voxel->picker, modifier1, modifier2);
        }
    }

    if ((mouseButtons[0] & MOUSE_BUTTON_RIGHT) != (mouseButtons[1] & MOUSE_BUTTON_RIGHT)) {
        if (!(mouseButtons[0] & MOUSE_BUTTON_LEFT)) {
            voxel->panelManager.dragging = 0;
        }

        Panel* panel = panel_manager_find_panel(&voxel->panelManager, mouseX[0], mouseY[0]);

        if (panel) {

        } else if (mouseButtons[0] & MOUSE_BUTTON_RIGHT) {

        } else {
            voxel->picker.selection.rotation = (voxel->picker.selection.rotation + 1) % 4;
        }
    }

    return 1;
}

void texture_panel(void* panelPtr, void* userData) {
    Panel* panel = (Panel*)panelPtr;

    panel->drawCallback(panel->owner);
    panel_texture(panel);
}

void voxel_draw(Voxel* voxel) {
    world_update(&voxel->world, &voxel->camera);

    if (!voxel->vulkan) {
        renderer_clear(&voxel->renderer);
        renderer_render_world(&voxel->renderer, &voxel->world, &voxel->camera);
        renderer_render_picker(&voxel->renderer, &voxel->picker);
        renderer_render_panels(&voxel->renderer, &voxel->panelManager.panels);
    } else {
        linked_list_foreach(&voxel->panelManager.panels, texture_panel, voxel);
        renderer_vulkan_render(&voxel->renderer, &voxel->world, &voxel->camera, &voxel->picker, &voxel->panelManager.panels);       
    }
    struct timeval oldFrameTime = voxel->frameTime;
    gettimeofday(&voxel->frameTime, NULL);

    static int i = 0;
    if ((i = (i + 1) % 10) == 0) {
        struct timeval elapsed;
        timersub(&voxel->frameTime, &oldFrameTime, &elapsed);
        long millisElapsed = (elapsed.tv_sec * 1000000 + elapsed.tv_usec) / 1000;

        fps_panel_set_fps(&voxel->fpsPanel, 1000.0 / millisElapsed);
    }
}

void voxel_setup_vulkan(Voxel* voxel) {
    voxel->vulkan = NULL;

    if (!glfwVulkanSupported() || getenv("FORCE_OPENGL")) {
        printf("Using OpenGL.\n");
        return;
    }

    Vulkan* vulkan = NEW(Vulkan, 1);
    if (!vulkan_create_instance("Voxel", &vulkan->instance)) {
        printf("failed to create Vulkan instance.\n");
        printf("Using OpenGL.\n");
        return;
    }

    if (glfwCreateWindowSurface(vulkan->instance, voxel->window.glfwWindow, NULL, &vulkan->surface) != VK_SUCCESS) {
        printf("failed to create window surface.\n");
        printf("Using OpenGL.\n");
        return;
    }

    voxel->window.surface = vulkan->surface;

    vulkan_pick_physical_device(vulkan->instance, voxel->window.surface, &vulkan->physicalDevice);

    vulkan_create_logical_device(vulkan->physicalDevice, voxel->window.surface, &vulkan->device, &voxel->renderer.renderState.vulkan.graphicsQueue, &voxel->renderer.renderState.vulkan.presentQueue);
    vulkan->commandQueue = voxel->renderer.renderState.vulkan.graphicsQueue;

    vulkan_create_command_pool(vulkan->physicalDevice, vulkan->device, vulkan->surface, &vulkan->commandPool);

    voxel->vulkan = vulkan;

    printf("Using Vulkan.\n");
}

void voxel_teardown_vulkan(Voxel* voxel) {
    PFN_vkDestroyCommandPool pfnDestroyCommandPool =
        (PFN_vkDestroyCommandPool)glfwGetInstanceProcAddress(NULL, "vkDestroyCommandPool");
    PFN_vkDestroyDevice pfnDestroyDevice =
        (PFN_vkDestroyDevice)glfwGetInstanceProcAddress(NULL, "vkDestroyDevice");
    PFN_vkDestroySurfaceKHR pfnDestroySurfaceKHR =
        (PFN_vkDestroySurfaceKHR)glfwGetInstanceProcAddress(NULL, "vkDestroySurfaceKHR");
    PFN_vkDestroyInstance pfnDestroyInstance =
        (PFN_vkDestroyInstance)glfwGetInstanceProcAddress(voxel->vulkan->instance, "vkDestroyInstance");

    pfnDestroyCommandPool(voxel->vulkan->device, voxel->vulkan->commandPool, NULL);
    pfnDestroyDevice(voxel->vulkan->device, NULL);
    pfnDestroySurfaceKHR(voxel->vulkan->instance, voxel->window.surface, NULL);
    pfnDestroyInstance(voxel->vulkan->instance, NULL);
}

void voxel_setup(Application* application) {
    Voxel* voxel = (Voxel*)application->owner;

    g_resources_register(resources_get_resource());

    voxel_setup_vulkan(voxel);

    renderer_init(&voxel->renderer, &voxel->window, voxel->vulkan);

    camera_init(&voxel->camera, voxel->vulkan);
    camera_move(&voxel->camera, Y, 2);

    world_init(&voxel->world, voxel->vulkan, "cubes");

    picker_init(&voxel->picker, &voxel->world, &voxel->undoStack);

    panel_manager_init(&voxel->panelManager);

    fps_panel_init(&voxel->fpsPanel, &voxel->renderer, &voxel->panelManager);

#ifdef __APPLE__
    fps_panel_set_position(&voxel->fpsPanel, 16, application->window->height - 30);
#else
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(primary, &xscale, &yscale);

    fps_panel_set_position(&voxel->fpsPanel, 16 / xscale, (application->window->height / yscale) - 30);
#endif

    picker_panel_init(&voxel->pickerPanel, &voxel->renderer, &voxel->panelManager, &voxel->picker);

    undo_stack_init(&voxel->undoStack);

    voxel_resize(application);
}

void voxel_main(Application* application) {
    Voxel* voxel = (Voxel*)application->owner;

    while (!glfwWindowShouldClose(application->window->glfwWindow))
    {
        glfwPollEvents();
        if (!voxel_process_input(voxel)) {
            break;
        }

        voxel_draw(voxel);

        if (!voxel->vulkan) {
            glfwSwapBuffers(application->window->glfwWindow);
        }
    }
}

void voxel_resize(Application* application) {
    Voxel* voxel = (Voxel*)application->owner;

#ifdef __APPLE__
    fps_panel_set_position(&voxel->fpsPanel, 16, application->window->height - 30);
#else
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(primary, &xscale, &yscale);

    fps_panel_set_position(&voxel->fpsPanel, 16 / xscale, (application->window->height / yscale) - 30);
#endif

    camera_set_aspect(&voxel->camera, (float)application->window->width / application->window->height);

    if (!voxel->vulkan) {
        renderer_resize(&voxel->renderer, application->window->width, application->window->height, &voxel->camera);
    } else {
        renderer_vulkan_resize(&voxel->renderer);
    }
}

void voxel_teardown(Application* application) {
    Voxel* voxel = (Voxel*)application->owner;

    if (voxel->vulkan) {
        PFN_vkDeviceWaitIdle pfnDeviceWaitIdle =
            (PFN_vkDeviceWaitIdle)glfwGetInstanceProcAddress(NULL, "vkDeviceWaitIdle");

        pfnDeviceWaitIdle(voxel->vulkan->device);
    }

    undo_stack_destroy(&voxel->undoStack);
    world_destroy(&voxel->world);
    fps_panel_destroy(&voxel->fpsPanel);
    picker_panel_destroy(&voxel->pickerPanel);
    panel_manager_destroy(&voxel->panelManager);
    picker_destroy(&voxel->picker);
    renderer_destroy(&voxel->renderer);

    if (voxel->vulkan) {
        voxel_teardown_vulkan(voxel);
    }
}

void voxel_run(Voxel* voxel) {
    window_open(&voxel->window);
}