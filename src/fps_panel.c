#include "fps_panel.h"
#include "internal/fps_panel.h"

FPSPanel* fps_panel_init(FPSPanel* p, PanelManager* panelManager) {
    FPSPanel* fpsPanel = p ? p : NEW(FPSPanel, 1);

    panel_init(&fpsPanel->panel, fpsPanel, fps_panel_draw, panelManager, FPS_PANEL_WIDTH, FPS_PANEL_HEIGHT);

    return fpsPanel;
}

void fps_panel_destroy(FPSPanel* fpsPanel) {
    panel_destroy(&fpsPanel->panel);
}

void fps_panel_set_fps(FPSPanel* fpsPanel, double fps) {
    fpsPanel->fps = fps;
}

void fps_panel_draw(void* fpsPanelPtr) {
    FPSPanel* fpsPanel = (FPSPanel*)fpsPanelPtr;

    char fpsStr[12];
    sprintf(fpsStr, "%.2f FPS", fpsPanel->fps);

    cairo_set_source_rgba(fpsPanel->panel.cr, 0, 0, 0, 0);
    cairo_set_operator(fpsPanel->panel.cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(fpsPanel->panel.cr);
    cairo_set_operator(fpsPanel->panel.cr, CAIRO_OPERATOR_OVER);

    cairo_set_source_rgb(fpsPanel->panel.cr, 0, 0, 0);

    cairo_select_font_face(fpsPanel->panel.cr, "Cantarell Regular", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(fpsPanel->panel.cr, 12);

    cairo_move_to(fpsPanel->panel.cr, 0, 15);
    cairo_show_text(fpsPanel->panel.cr, fpsStr);
}

void fps_panel_set_position(FPSPanel* fpsPanel, unsigned int x, unsigned int y) {
    panel_set_position(&fpsPanel->panel, x, y);
}
