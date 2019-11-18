#include "mouse.h"

Mouse* mouse_init(Mouse* m, const char* devicePath) {
    Mouse* mouse = m ? m : NEW(Mouse, 1);

    memset(mouse, 0, sizeof(Mouse));

    mouse->fd = open(devicePath, O_RDONLY | O_NONBLOCK);
    if (mouse->fd == -1) {
        perror("Mouse: unable to open device.");
        return NULL;
    }

    mouse->bounds.left = -1;
    mouse->bounds.right = -1;
    mouse->bounds.top = -1;
    mouse->bounds.bottom = -1;

    return mouse;
}

void mouse_destroy(Mouse* mouse) {
    close(mouse->fd);
}

void mouse_set_bounds(Mouse* mouse, int left, int right, int top, int bottom) {
    mouse->bounds.left = left;
    mouse->bounds.right = right;
    mouse->bounds.top = top;
    mouse->bounds.bottom = bottom;
}

void mouse_process_events(Mouse* mouse) {
    int n;
    struct input_event inputEvent;
    while ((n = read(mouse->fd, (void*)&inputEvent, sizeof(struct input_event))) > 0) {
        if (inputEvent.type == EV_REL) {     
            if (inputEvent.code == 0) {
                int x = mouse->x + inputEvent.value;
                if (mouse->bounds.left < 0 || x >= mouse->bounds.left) {
                    if (mouse->bounds.right < 0 || x < mouse->bounds.right) {
                        mouse->x = x;
                    } else {
                        mouse->x = mouse->bounds.right;
                    }
                } else {
                    mouse->x = mouse->bounds.left;
                }
            } else {
                int y = mouse->y + inputEvent.value;
                if (mouse->bounds.top < 0 || y >= mouse->bounds.top) {
                    if (mouse->bounds.bottom < 0 || y < mouse->bounds.bottom) {
                        mouse->y = y;
                    } else {
                        mouse->y = mouse->bounds.bottom;
                    }
                } else {
                    mouse->y = mouse->bounds.top;
                }
            }
        } else if (inputEvent.type == EV_KEY && inputEvent.value != 2) {     
            if (inputEvent.code == BTN_LEFT) {
                if (inputEvent.value) {
                    mouse->buttons |= BUTTON_LEFT;
                } else {
                    mouse->buttons &= ~BUTTON_LEFT;
                }
            } else if (inputEvent.code == BTN_RIGHT) {
                if (inputEvent.value) {
                    mouse->buttons |= BUTTON_RIGHT;
                } else {
                    mouse->buttons &= ~BUTTON_RIGHT;
                }
            }
        }
    }
}

char mouse_state(Mouse* mouse, int* x, int* y) {
    mouse_process_events(mouse);
    if (x) *x = mouse->x;
    if (y) *y = mouse->y;
        
    return mouse->buttons;
}

