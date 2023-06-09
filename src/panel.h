#ifndef PANEL_H
#define PANEL_H

#include <cairo/cairo.h>
#include <stdlib.h>

#include "global.h"
#include "linked_list.h"
#include "window.h"

struct Panel;
struct PanelManager;

typedef struct Panel {
    unsigned int vbo;
    unsigned int tex;

    cairo_surface_t *surface;
    cairo_t *cr;

    void* owner;
    void (*drawCallback)(void*);
    struct PanelManager* manager;

    unsigned int position[2];
    unsigned int width;
    unsigned int height;
} Panel;

Panel* panel_init(Panel* d, void* owner, void (*drawCallback)(void*), struct PanelManager* manager, unsigned int width, unsigned int height);
void panel_destroy(Panel* panel);

void panel_set_position(Panel* panel, int x, int y);
void panel_translate(Panel* panel, int x, int y);

void panel_texture(Panel* panel);

/* PanelManager */

typedef struct PanelManager {
    LinkedList panels;
    Panel* active_panel;
    char dragging;
} PanelManager;

PanelManager* panel_manager_init(PanelManager* pm);
void panel_manager_destroy(PanelManager* panelManager);
void panel_manager_add_panel(PanelManager* panelManager, Panel* panel);
Panel* panel_manager_find_panel(PanelManager* panelManager, unsigned int x, unsigned int y);

#endif // PANEL_H
