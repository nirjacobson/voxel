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
        camera_rotate(&voxel->camera, voxel->camera.right, 0.05);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_LEFT))
        camera_rotate(&voxel->camera, Y, 0.05);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_DOWN))
        camera_rotate(&voxel->camera, voxel->camera.right, -0.05);

    if (window_key_is_pressed(&voxel->window, GLFW_KEY_RIGHT))
        camera_rotate(&voxel->camera, Y, -0.05);

    renderer_apply_camera(&voxel->renderer, &voxel->camera);

    f[1] = f[0];
    f[0] = window_key_is_pressed(&voxel->window, GLFW_KEY_F);
    if (!f[1] && f[0]) {
        window_toggle_fullscreen(&voxel->window);
    }

    return 1;
}

void voxel_draw(Voxel* voxel) {
    renderer_clear(&voxel->renderer);
    renderer_render_world(&voxel->renderer, &voxel->world, &voxel->camera);
}


void voxel_setup(Application* application) {
    Voxel* voxel = (Voxel*)application->owner;

    renderer_init(&voxel->renderer);
    camera_init(&voxel->camera);

    camera_move(&voxel->camera, Y, 2);

    voxel_resize(application);

    world_init(&voxel->world, "cubes");
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
    Voxel* voxel = (Voxel*)application->owner;

    camera_set_aspect(&voxel->camera, (double)application->window->width / application->window->height);

    renderer_resize(&voxel->renderer, application->window->width, application->window->height, &voxel->camera);
}

void voxel_teardown(Application* application) {
    Voxel* voxel = (Voxel*)application->owner;

    world_destroy(&voxel->world);
    renderer_destroy(&voxel->renderer);
}

void voxel_run(Voxel* voxel) {
    window_open(&voxel->window);
}