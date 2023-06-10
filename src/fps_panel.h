#ifndef FPS_PANEL_H
#define FPS_PANEL_H

#define FPS_PANEL_WIDTH     128
#define FPS_PANEL_HEIGHT     16

#include <stdio.h>

#include "panel.h"

typedef struct {
    Panel panel;
    float fps;
} FPSPanel;

FPSPanel* fps_panel_init(FPSPanel* p, PanelManager* panelManager);
void fps_panel_destroy(FPSPanel* fpsPanel);

void fps_panel_set_fps(FPSPanel* fpsPanel, float fps);
void fps_panel_set_position(FPSPanel* fpsPanel, unsigned int x, unsigned int y);

#endif // FPS_PANEL_H
