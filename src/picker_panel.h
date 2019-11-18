#ifndef PICKER_PANEL_H
#define PICKER_PANEL_H

#define PICKER_PANEL_WIDTH          320
#define PICKER_PANEL_HEIGHT         176

#define PENCIL_BUTTON_X                    8
#define PENCIL_BUTTON_Y                   24
#define ERASER_BUTTON_X                   46
#define ERASER_BUTTON_Y                   24
#define COLOR_DROPPER_BUTTON_X             8
#define COLOR_DROPPER_BUTTON_Y            62
#define SELECT_BUTTON_X                    8
#define SELECT_BUTTON_Y                  135
#define STAMP_BUTTON_X                    46
#define STAMP_BUTTON_Y                   135
#define MOVE_BUTTON_X                     84
#define MOVE_BUTTON_Y                    135
#define BUTTON_WIDTH                      34
#define BUTTON_HEIGHT                     34
#define BLUE_BAR_X                       191
#define BLUE_BAR_Y                        24
#define PALETTE_X                        191
#define PALETTE_Y                         48

#include "panel.h"
#include "picker.h"

typedef struct {
  Panel panel;

  cairo_surface_t* background_surface;
  cairo_surface_t* pencil_button_surface_natural;
  cairo_surface_t* pencil_button_surface_selected;
  cairo_surface_t* eraser_button_surface_natural;
  cairo_surface_t* eraser_button_surface_selected;
  cairo_surface_t* dropper_button_surface_natural;
  cairo_surface_t* dropper_button_surface_selected;
  cairo_surface_t* select_button_surface_natural;
  cairo_surface_t* select_button_surface_selected;
  cairo_surface_t* stamp_button_surface_natural;
  cairo_surface_t* stamp_button_surface_selected;
  cairo_surface_t* move_button_surface_natural;
  cairo_surface_t* move_button_surface_selected;

  Picker* picker;
} PickerPanel;

PickerPanel* picker_panel_init(PickerPanel* td, PanelManager* panelManager, Picker* picker);
void picker_panel_destroy(PickerPanel* pickerPanel);

void picker_panel_draw_border(PickerPanel* pickerPanel);
void picker_panel_draw_background(PickerPanel* pickerPanel);
void picker_panel_draw_titlebar(PickerPanel* pickerPanel);
void picker_panel_draw_bluebar(PickerPanel* pickerPanel);
void picker_panel_draw_palette(PickerPanel* pickerPanel);

void picker_panel_draw_pencil_button(PickerPanel* pickerPanel);
void picker_panel_draw_eraser_button(PickerPanel* pickerPanel);
void picker_panel_draw_dropper_button(PickerPanel* pickerPanel);
void picker_panel_draw_select_button(PickerPanel* pickerPanel);
void picker_panel_draw_stamp_button(PickerPanel* pickerPanel);
void picker_panel_draw_move_button(PickerPanel* pickerPanel);
void picker_panel_draw_buttons(PickerPanel* pickerPanel);
void picker_panel_draw(void* pickerPanelPtr);

void picker_panel_add_titlebar_action_region(PickerPanel* pickerPanel);
void picker_panel_add_bluebar_action_region(PickerPanel* pickerPanel);
void picker_panel_add_palette_action_region(PickerPanel* pickerPanel);
void picker_panel_add_pencil_button_action_region(PickerPanel* pickerPanel);
void picker_panel_add_eraser_button_action_region(PickerPanel* pickerPanel);
void picker_panel_add_dropper_button_action_region(PickerPanel* pickerPanel);
void picker_panel_add_select_button_action_region(PickerPanel* pickerPanel);
void picker_panel_add_stamp_button_action_region(PickerPanel* pickerPanel);
void picker_panel_add_move_button_action_region(PickerPanel* pickerPanel);

#endif // PICKER_PANEL_H
