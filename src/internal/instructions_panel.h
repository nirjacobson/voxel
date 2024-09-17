#ifndef INSTRUCTIONS_PANEL_INTERNAL_H
#define INSTRUCTIONS_PANEL_INTERNAL_H

#include "../instructions_panel.h"

#define PENCIL_BUTTON_X                    8
#define PENCIL_BUTTON_Y                   24
#define COLOR_DROPPER_BUTTON_X             8
#define COLOR_DROPPER_BUTTON_Y            74
#define ERASER_BUTTON_X                    8
#define ERASER_BUTTON_Y                  124
#define SELECT_BUTTON_X                  185
#define SELECT_BUTTON_Y                   24
#define STAMP_BUTTON_X                   185
#define STAMP_BUTTON_Y                    74
#define MOVE_BUTTON_X                    185
#define MOVE_BUTTON_Y                    124

/* Action region callbacks */

void instructions_panel_titlebar_press(ActionRegionArgs* args);
void instructions_panel_titlebar_release(ActionRegionArgs* args);

void instructions_panel_close_button_release(ActionRegionArgs* args);

void instructions_panel_ok_button_press(ActionRegionArgs* args);
void instructions_panel_ok_button_release(ActionRegionArgs* args);

/* InstructionsPanel */

void instructions_panel_draw_background(InstructionsPanel* instrPanel);
void instructions_panel_draw_titlebar(InstructionsPanel* instrPanel);
void instructions_panel_draw_title_buttons(InstructionsPanel* instrPanel);
void instructions_panel_draw_image(InstructionsPanel* instrPanel);
void instructions_panel_draw_image(InstructionsPanel* instrPanel);
void instructions_panel_draw_dialog_buttons(InstructionsPanel* instrPanel);

void instructions_panel_draw_pencil_button(InstructionsPanel* pickerPanel);
void instructions_panel_draw_eraser_button(InstructionsPanel* pickerPanel);
void instructions_panel_draw_dropper_button(InstructionsPanel* pickerPanel);
void instructions_panel_draw_select_button(InstructionsPanel* pickerPanel);
void instructions_panel_draw_stamp_button(InstructionsPanel* pickerPanel);
void instructions_panel_draw_move_button(InstructionsPanel* pickerPanel);
void instructions_panel_draw_buttons(InstructionsPanel* pickerPanel);

void instructions_panel_draw(void* instrPanelPtr);

void instructions_panel_add_titlebar_action_region(InstructionsPanel* instrPanel);
void instructions_panel_add_close_button_action_region(InstructionsPanel* instrPanel);
void instructions_panel_add_ok_button_action_region(InstructionsPanel* instrPanel);

#endif