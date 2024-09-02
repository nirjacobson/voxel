#ifndef PANEL_INTERNAL_H
#define PANEL_INTERNAL_H

#include "../panel.h"

/* Linked list processing callbacks */

char coords_over_action_region(void* coordsPtr, void* actionRegionPtr);
char coords_over_panel(void* coordsPtr, void* panelPtr);

void panel_create_vulkan_resources(Panel* panel);

#endif // PANEL_INTERNAL_H
