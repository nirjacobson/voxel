#include "voxel.h"

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

    if (keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_ESCAPE))
        return 0;

    mouseX[1] = mouseX[0];
    mouseY[1] = mouseY[0];
    mouseButtons[1] = mouseButtons[0];
    mouseButtons[0] = mouse_state(&voxel->mouse, &mouseX[0], &mouseY[0]);

    if (keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_W))
        camera_move(&voxel->camera, voxel->camera.forward, 0.5);
    
    if (keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_A))
        camera_move(&voxel->camera, voxel->camera.right, -0.5);
    
    if (keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_S))
        camera_move(&voxel->camera, voxel->camera.forward, -0.5);
    
    if (keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_D))
        camera_move(&voxel->camera, voxel->camera.right, 0.5);
    
    if (keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_UP))
        camera_rotate(&voxel->camera, voxel->camera.right, 0.05);
    
    if (keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_LEFT))
        camera_rotate(&voxel->camera, Y, 0.05);
    
    if (keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_DOWN))
        camera_rotate(&voxel->camera, voxel->camera.right, -0.05);
    
    if (keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_RIGHT))
        camera_rotate(&voxel->camera, Y, -0.05);
        
    camera_apply(&voxel->camera, &voxel->renderer);

    if (keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_TAB)) {
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
    
    if(keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_1))
        picker_set_action(&voxel->picker, PICKER_SET);
    
    if(keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_2))
        picker_set_action(&voxel->picker, PICKER_CLEAR);
    
    if(keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_Q))
        picker_set_action(&voxel->picker, PICKER_EYEDROPPER);
    
    if(keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_Z))
        picker_set_action(&voxel->picker, PICKER_SELECT);
    
    if(keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_X))
        picker_set_action(&voxel->picker, PICKER_STAMP);
    
    if(keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_C))
        picker_set_action(&voxel->picker, PICKER_MOVE);

    f[1] = f[0];
    f[0] = keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_F);
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

    if ((mouseButtons[0] & BUTTON_LEFT) != (mouseButtons[1] & BUTTON_LEFT)) {
        if (!(mouseButtons[0] & BUTTON_LEFT)) {
            voxel->panelManager.dragging = 0;
        }

        Panel* panel = panel_manager_find_panel(&voxel->panelManager, mouseX[0], mouseY[0]);

        if (panel) {
            GLuint action = (mouseButtons[0] & BUTTON_LEFT) ? MOUSE_PRESS : MOUSE_RELEASE;
            panel_action(panel, action, mouseX[0] - panel->position[0], mouseY[0] - panel->position[1]);
        } else if (mouseButtons[0] & BUTTON_LEFT) {
            char modifier1 = keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_LEFT_SHIFT);
            char modifier2 = keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_LEFT_SUPER);
            picker_press(&voxel->picker, modifier1, modifier2);
        } else {
            char modifier1 = keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_LEFT_SHIFT);
            char modifier2 = keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_LEFT_SUPER);
            picker_release(&voxel->picker, modifier1, modifier2);
        }
    }

    if ((mouseButtons[0] & BUTTON_RIGHT) != (mouseButtons[1] & BUTTON_RIGHT)) {
        if (!(mouseButtons[0] & BUTTON_LEFT)) {
            voxel->panelManager.dragging = 0;
        }

        Panel* panel = panel_manager_find_panel(&voxel->panelManager, mouseX[0], mouseY[0]);

        if (panel) {

        } else if (mouseButtons[0] & BUTTON_RIGHT) {

        } else {
            voxel->picker.selection.rotation = (voxel->picker.selection.rotation + 1) % 4;
        }
    }

    return 1; 
}

void voxel_draw(Voxel* voxel) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    world_draw(&voxel->world, &voxel->camera, &voxel->renderer);
    picker_draw(&voxel->picker, &voxel->renderer);
    panel_manager_draw(&voxel->panelManager);

    struct timeval oldFrameTime = voxel->frameTime;
    gettimeofday(&voxel->frameTime, NULL);

    struct timeval elapsed;
    timersub(&voxel->frameTime, &oldFrameTime, &elapsed);
    long millisElapsed = (elapsed.tv_sec * 1000000 + elapsed.tv_usec) / 1000;

    fps_panel_set_fps(&voxel->fpsPanel, 1000.0 / millisElapsed);
}


void voxel_setup(Application* application) {
    keyboard_init(&application->voxel->keyboard, application->window);
    mouse_init(&application->voxel->mouse, application->window);

    renderer_init(&application->voxel->renderer);
    camera_init(&application->voxel->camera);

    camera_move(&application->voxel->camera, Y, 2);

    voxel_resize(application);
    
    picker_init(&application->voxel->picker);

    panel_manager_init(&application->voxel->panelManager, &application->voxel->renderer);
    picker_panel_init(&application->voxel->pickerPanel, &application->voxel->panelManager, &application->voxel->picker);

    fps_panel_init(&application->voxel->fpsPanel, &application->voxel->panelManager);
    fps_panel_set_position(&application->voxel->fpsPanel, 16, application->window->height - 30);
    
    world_init(&application->voxel->world, "cubes");
    picker_set_world(&application->voxel->picker, &application->voxel->world);
}

void voxel_main(Application* application) {
    while (!glfwWindowShouldClose(application->window->glfwWindow))
    {
        if (!voxel_process_input(application->voxel))
            break;

        voxel_draw(application->voxel);

        glfwSwapBuffers(application->window->glfwWindow);
        glfwPollEvents();
    }
}

void voxel_resize(Application* application) {
    glViewport(0, 0, application->window->width, application->window->height);

    GLfloat mat[16];
    mat4_orthographic(mat, 0, application->window->width, 0, application->window->height);
    renderer_2D_update_projection(&application->voxel->renderer, mat);

    camera_set_aspect(&application->voxel->camera, (double)application->window->width / application->window->height);
    camera_apply(&application->voxel->camera, &application->voxel->renderer);

    fps_panel_set_position(&application->voxel->fpsPanel, 16, application->window->height - 30);
}

void voxel_teardown(Application* application) {
    world_destroy(&application->voxel->world);
    fps_panel_destroy(&application->voxel->fpsPanel);
    picker_panel_destroy(&application->voxel->pickerPanel);
    panel_manager_destroy(&application->voxel->panelManager);
    picker_destroy(&application->voxel->picker);
    renderer_destroy(&application->voxel->renderer);
}

void voxel_run(Voxel* voxel) {
    window_open(&voxel->window);
}