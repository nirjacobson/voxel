#include "voxel.h"
#include "internal/voxel.h"

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

    if (keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_ESCAPE))
        return 0;

    mouseX[1] = mouseX[0];
    mouseY[1] = mouseY[0];
    mouseButtons[1] = mouseButtons[0];
    mouseButtons[0] = mouse_state(&voxel->mouse, &mouseX[0], &mouseY[0]);

    f[1] = f[0];
    f[0] = keyboard_key_is_pressed(&voxel->keyboard, GLFW_KEY_F);
    if (!f[1] && f[0]) {
        window_toggle_fullscreen(&voxel->window);
    }

    if (mouseX[0] != mouseX[1] || mouseY[0] != mouseY[1]) {
        // ...
    }

    if ((mouseButtons[0] & BUTTON_LEFT) != (mouseButtons[1] & BUTTON_LEFT)) {
        // ...
    }

    if ((mouseButtons[0] & BUTTON_RIGHT) != (mouseButtons[1] & BUTTON_RIGHT)) {
        // ...
    }

    return 1;
}

void voxel_draw(Voxel* voxel) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void voxel_setup(Application* application) {
    Voxel* voxel = (Voxel*)application->owner;

    glClearColor(0.50f, 0.75f, 0.86f, 1.0f);
    glClearDepthf(1);

    keyboard_init(&voxel->keyboard, application->window);
    mouse_init(&voxel->mouse, application->window);
}

void voxel_main(Application* application) {
    Voxel* voxel = (Voxel*)application->owner;

    while (!glfwWindowShouldClose(application->window->glfwWindow))
    {
        if (!voxel_process_input(voxel))
            break;

        voxel_draw(voxel);

        glfwSwapBuffers(application->window->glfwWindow);
        glfwPollEvents();
    }
}

void voxel_resize(Application* application) {
    glViewport(0, 0, application->window->width, application->window->height);
}

void voxel_teardown(Application* application) {
    // ...
}

void voxel_run(Voxel* voxel) {
    window_open(&voxel->window);
}