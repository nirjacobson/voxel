#include "voxel.h"

Voxel* voxel_init(Voxel* v) {
    Voxel* voxel = v ? v : NEW(Voxel, 1);
    
    keyboard_init(&voxel->keyboard, "/dev/input/event1");
    window_init(&voxel->window);
    mouse_init(&voxel->mouse, "/dev/input/event0");
    mouse_set_bounds(&voxel->mouse, 0, voxel->window.width, 0, voxel->window.height);

    cursor_init(&voxel->cursor);

    renderer_init(&voxel->renderer);
    camera_init(&voxel->camera);

    camera_set_aspect(&voxel->camera, (double)voxel->window.width / voxel->window.height);
    camera_move(&voxel->camera, Y, 2);
    camera_apply(&voxel->camera, &voxel->renderer);

    float projection2D[16];
    mat4_orthographic(projection2D, 0, voxel->window.width, 0, voxel->window.height);
    renderer_2D_update_projection(&voxel->renderer, projection2D);

    picker_init(&voxel->picker);

    panel_manager_init(&voxel->panelManager, &voxel->renderer);
    picker_panel_init(&voxel->pickerPanel, &voxel->panelManager, &voxel->picker);

    return voxel;
}

void voxel_destroy(Voxel* voxel) {
    picker_panel_destroy(&voxel->pickerPanel);
    panel_manager_destroy(&voxel->panelManager);
    picker_destroy(&voxel->picker);
    renderer_destroy(&voxel->renderer);
    cursor_destroy(&voxel->cursor);
    mouse_destroy(&voxel->mouse);
    keyboard_destroy(&voxel->keyboard);
}

char voxel_process_input(Voxel* voxel) {
    static int mouseX[2];
    static int mouseY[2];
    static char mouseButtons[2];

    static int tab = 0;

    if (keyboard_key_is_pressed(&voxel->keyboard, KEY_ESC))
        return 0;

    mouseX[1] = mouseX[0];
    mouseY[1] = mouseY[0];
    mouseButtons[1] = mouseButtons[0];
    mouseButtons[0] = mouse_state(&voxel->mouse, &mouseX[0], &mouseY[0]);

    if (keyboard_key_is_pressed(&voxel->keyboard, KEY_W))
        camera_move(&voxel->camera, voxel->camera.forward, 0.5);
    
    if (keyboard_key_is_pressed(&voxel->keyboard, KEY_A))
        camera_move(&voxel->camera, voxel->camera.right, -0.5);
    
    if (keyboard_key_is_pressed(&voxel->keyboard, KEY_S))
        camera_move(&voxel->camera, voxel->camera.forward, -0.5);
    
    if (keyboard_key_is_pressed(&voxel->keyboard, KEY_D))
        camera_move(&voxel->camera, voxel->camera.right, 0.5);
    
    if (keyboard_key_is_pressed(&voxel->keyboard, KEY_UP))
        camera_rotate(&voxel->camera, voxel->camera.right, 0.05);
    
    if (keyboard_key_is_pressed(&voxel->keyboard, KEY_LEFT))
        camera_rotate(&voxel->camera, Y, 0.05);
    
    if (keyboard_key_is_pressed(&voxel->keyboard, KEY_DOWN))
        camera_rotate(&voxel->camera, voxel->camera.right, -0.05);
    
    if (keyboard_key_is_pressed(&voxel->keyboard, KEY_RIGHT))
        camera_rotate(&voxel->camera, Y, -0.05);
        
    camera_apply(&voxel->camera, &voxel->renderer);

    if (keyboard_key_is_pressed(&voxel->keyboard, KEY_TAB)) {
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
    
    if(keyboard_key_is_pressed(&voxel->keyboard, KEY_1))
        picker_set_action(&voxel->picker, PICKER_SET);
    
    if(keyboard_key_is_pressed(&voxel->keyboard, KEY_2))
        picker_set_action(&voxel->picker, PICKER_CLEAR);
    
    if(keyboard_key_is_pressed(&voxel->keyboard, KEY_Q))
        picker_set_action(&voxel->picker, PICKER_EYEDROPPER);
    
    if(keyboard_key_is_pressed(&voxel->keyboard, KEY_Z))
        picker_set_action(&voxel->picker, PICKER_SELECT);
    
    if(keyboard_key_is_pressed(&voxel->keyboard, KEY_X))
        picker_set_action(&voxel->picker, PICKER_STAMP);
    
    if(keyboard_key_is_pressed(&voxel->keyboard, KEY_C))
        picker_set_action(&voxel->picker, PICKER_MOVE);

    if (mouseX[0] != mouseX[1] || mouseY[0] != mouseY[1]) {
        cursor_set_position(&voxel->cursor, mouseX[0], mouseY[0]);

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
            char modifier1 = keyboard_key_is_pressed(&voxel->keyboard, KEY_LEFTSHIFT);
            char modifier2 = keyboard_key_is_pressed(&voxel->keyboard, KEY_LEFTALT);
            picker_press(&voxel->picker, modifier1, modifier2);
        } else {
            char modifier1 = keyboard_key_is_pressed(&voxel->keyboard, KEY_LEFTSHIFT);
            char modifier2 = keyboard_key_is_pressed(&voxel->keyboard, KEY_LEFTALT);
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

void voxel_run(Voxel* voxel) {
    World world;
    world_init(&world, "cubes");

    picker_set_world(&voxel->picker, &world);

    while (1) {
        if (!voxel_process_input(voxel))
            break;

        world_draw(&world, &voxel->camera, &voxel->renderer);

        picker_draw(&voxel->picker, &voxel->renderer);

        panel_manager_draw(&voxel->panelManager);

        cursor_draw(&voxel->cursor, &voxel->renderer);

        window_draw(&voxel->window);
    }

    world_destroy(&world);
}