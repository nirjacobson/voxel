#ifndef PANEL_INTERNAL_H
#define PANEL_INTERNAL_H

#include "../panel.h"

void panel_texture(Panel* panel);
void panel_draw(Panel* panel);

/* Linked list processing callbacks */

char coords_over_action_region(void* coordsPtr, void* actionRegionPtr);
char coords_over_panel(void* coordsPtr, void* panelPtr);
void draw_panel(void* panelPtr, void* unusued);

#endif // PANEL_INTERNAL_H
