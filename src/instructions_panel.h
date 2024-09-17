#ifndef INSTRUCTIONS_PANEL_H
#define INSTRUCTIONS_PANEL_H

#include <gio/gio.h>

#include "panel.h"

typedef struct {
    Panel panel;

    cairo_surface_t* image_surface;

    cairo_surface_t* pencil_button_surface;
    cairo_surface_t* eraser_button_surface;
    cairo_surface_t* dropper_button_surface;
    cairo_surface_t* select_button_surface;
    cairo_surface_t* stamp_button_surface;
    cairo_surface_t* move_button_surface;

    bool ok_highlighted;

    int width;
    int height;
} InstructionsPanel;

InstructionsPanel* instructions_panel_init(InstructionsPanel* ip, Renderer* renderer, PanelManager* panelManager);
void instructions_panel_destroy(InstructionsPanel* instrPanel);

void instructions_panel_set_position(InstructionsPanel* instrPanel, unsigned int x, unsigned int y);

#endif // INSTRUCTIONS_PANEL_H