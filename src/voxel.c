#include "voxel.h"
#include "internal/voxel.h"

extern const bool enableValidationLayers;

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

    static int tab = 0;

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_ESCAPE))
        return 0;

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
        camera_rotate(&voxel->camera, voxel->camera.right, -0.05);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_LEFT))
        camera_rotate(&voxel->camera, Y, -0.05);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_DOWN))
        camera_rotate(&voxel->camera, voxel->camera.right, 0.05);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_RIGHT))
        camera_rotate(&voxel->camera, Y, 0.05);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_TAB)) {
        if (!tab) {
            tab = 1;
            voxel->picker.mode = voxel->picker.mode == PICKER_ONTO ? PICKER_ADJACENT : PICKER_ONTO;
            float nx = (2.0 * ((float)mouseX[0])/voxel->window.width) - 1;
            float ny = (2.0 * ((float)(voxel->window.height - mouseY[0]))/voxel->window.height) - 1;
            picker_update(&voxel->picker, &voxel->camera, nx, ny);
        }
    } else {
        tab = 0;
    }

    if(window_key_is_pressed(&voxel->window, GLFW_KEY_1))
        picker_set_action(&voxel->picker, PICKER_SET);

    if(window_key_is_pressed(&voxel->window, GLFW_KEY_2))
        picker_set_action(&voxel->picker, PICKER_CLEAR);

    if(window_key_is_pressed(&voxel->window, GLFW_KEY_Q))
        picker_set_action(&voxel->picker, PICKER_EYEDROPPER);

    if(window_key_is_pressed(&voxel->window, GLFW_KEY_Z))
        picker_set_action(&voxel->picker, PICKER_SELECT);

    if(window_key_is_pressed(&voxel->window, GLFW_KEY_X))
        picker_set_action(&voxel->picker, PICKER_STAMP);

    if(window_key_is_pressed(&voxel->window, GLFW_KEY_C))
        picker_set_action(&voxel->picker, PICKER_MOVE);

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
            panel_action(panel, action, mouseX[0] - panel->position[0], mouseY[0] - panel->position[1]);
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

    linked_list_foreach(&voxel->panelManager.panels, texture_panel, voxel);

    renderer_render(&voxel->renderer, &voxel->world, &voxel->camera, &voxel->picker, &voxel->panelManager.panels);

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
    vulkan_create_instance("Voxel", &voxel->vulkan.instance);

    if (glfwCreateWindowSurface(voxel->vulkan.instance, voxel->window.glfwWindow, NULL, &voxel->vulkan.surface) != VK_SUCCESS) {
        printf("failed to create window surface.\n");
        assert(false);
    }

    voxel->window.surface = voxel->vulkan.surface;

    vulkan_pick_physical_device(voxel->vulkan.instance, voxel->window.surface, &voxel->vulkan.physicalDevice);

    vulkan_create_logical_device(voxel->vulkan.physicalDevice, voxel->window.surface, &voxel->vulkan.device, &voxel->renderer.graphicsQueue, &voxel->renderer.presentQueue);
    voxel->vulkan.commandQueue = voxel->renderer.graphicsQueue;

    vulkan_create_command_pool(voxel->vulkan.physicalDevice, voxel->vulkan.device, voxel->vulkan.surface, &voxel->vulkan.commandPool);
}

void voxel_teardown_vulkan(Voxel* voxel) {
    vkDestroyCommandPool(voxel->vulkan.device, voxel->vulkan.commandPool, NULL);
    vkDestroyDevice(voxel->vulkan.device, NULL);
    vkDestroySurfaceKHR(voxel->vulkan.instance, voxel->window.surface, NULL);
    vkDestroyInstance(voxel->vulkan.instance, NULL);
}

void voxel_setup(Application* application) {
    Voxel* voxel = (Voxel*)application->owner;

    voxel_setup_vulkan(voxel);

    renderer_init(&voxel->renderer, &voxel->window, &voxel->vulkan);
    camera_init(&voxel->camera);

    camera_move(&voxel->camera, Y, 2);

    world_init(&voxel->world, &voxel->vulkan, "cubes");

    picker_init(&voxel->picker, &voxel->world);

    panel_manager_init(&voxel->panelManager);

    fps_panel_init(&voxel->fpsPanel, &voxel->renderer, &voxel->panelManager);
    fps_panel_set_position(&voxel->fpsPanel, 16, application->window->height - 30);

    picker_panel_init(&voxel->pickerPanel, &voxel->renderer, &voxel->panelManager, &voxel->picker);

    voxel_resize(application);
}

void voxel_main(Application* application) {
    Voxel* voxel = (Voxel*)application->owner;

    while (!glfwWindowShouldClose(application->window->glfwWindow))
    {
        glfwPollEvents();
        if (!voxel_process_input(voxel))
            break;

        voxel_draw(voxel);
    }
}

void voxel_resize(Application* application) {
    Voxel* voxel = (Voxel*)application->owner;

    fps_panel_set_position(&voxel->fpsPanel, 16, application->window->height - 30);

    camera_set_aspect(&voxel->camera, (float)application->window->width / application->window->height);
    renderer_resize(&voxel->renderer);
}

void voxel_teardown(Application* application) {
    Voxel* voxel = (Voxel*)application->owner;

    vkDeviceWaitIdle(voxel->vulkan.device);

    world_destroy(&voxel->world);
    fps_panel_destroy(&voxel->fpsPanel);
    picker_panel_destroy(&voxel->pickerPanel);
    panel_manager_destroy(&voxel->panelManager);
    picker_destroy(&voxel->picker);
    renderer_destroy(&voxel->renderer);

    voxel_teardown_vulkan(voxel);
}

void voxel_run(Voxel* voxel) {
    window_open(&voxel->window);
}