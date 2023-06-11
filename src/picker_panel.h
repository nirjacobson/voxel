#ifndef PICKER_PANEL_H
#define PICKER_PANEL_H

#include "panel.h"
#include "picker.h"

typedef struct {
    Panel panel;

    cairo_surface_t* background_surface;
    cairo_surface_t* pencil_button_surface_natural;
    cairo_surface_t* pencil_button_surface_selected;
    cairo_surface_t* eraser_button_surface_natural;
    cairo_surface_t* eraser_button_surface_selected;

    Picker* picker;
} PickerPanel;

PickerPanel* picker_panel_init(PickerPanel* td, PanelManager* panelManager, Picker* picker);
void picker_panel_destroy(PickerPanel* pickerPanel);

#endif // PICKER_PANEL_H
