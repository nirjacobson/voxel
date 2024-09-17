#include "instructions_panel.h"
#include "internal/instructions_panel.h"

/* Action region callbacks */

void instructions_panel_titlebar_press(ActionRegionArgs* args) {
    args->panel->manager->dragging = 1;
}

void instructions_panel_titlebar_release(ActionRegionArgs* args) {
    args->panel->manager->dragging = 0;
}

void instructions_panel_close_button_release(ActionRegionArgs* args) {
    panel_manager_remove_panel(args->panel->manager, args->panel);
}

void instructions_panel_ok_button_press(ActionRegionArgs* args) {
    InstructionsPanel* instrPanel = (InstructionsPanel*)args->panel->owner;
    instrPanel->ok_highlighted = true;
    instructions_panel_draw_buttons(instrPanel);
}

void instructions_panel_ok_button_release(ActionRegionArgs* args) {
    InstructionsPanel* instrPanel = (InstructionsPanel*)args->panel->owner;
    instrPanel->ok_highlighted = false;
    instructions_panel_draw_buttons(instrPanel);
    panel_manager_remove_panel(args->panel->manager, args->panel);
}

extern cairo_status_t cairo_read_func(void* closure, unsigned char* data, unsigned int length);

InstructionsPanel* instructions_panel_init(InstructionsPanel* ip, Renderer* renderer, PanelManager* panelManager) {
    InstructionsPanel* instrPanel = ip ? ip : NEW(InstructionsPanel, 1);

    const char* paths[] = {
        "/img/instructions.png",
        "/img/pencil-natural.png",
        "/img/eraser-natural.png",
        "/img/color-dropper-natural.png",
        "/img/select-natural.png",
        "/img/stamp-natural.png",
        "/img/move-natural.png",
    };

    cairo_surface_t** surfaces[] = {
        &instrPanel->image_surface,
        &instrPanel->pencil_button_surface,
        &instrPanel->eraser_button_surface,
        &instrPanel->dropper_button_surface,
        &instrPanel->select_button_surface,
        &instrPanel->stamp_button_surface,
        &instrPanel->move_button_surface
    };

    int count = sizeof(surfaces) / sizeof(surfaces[0]);

    GInputStream* inputStream;

    for (int i = 0; i < count; i++) {
        inputStream = g_resources_open_stream(paths[i], G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
        *surfaces[i] = cairo_image_surface_create_from_png_stream(cairo_read_func, (void*)inputStream);
    }

    int width = cairo_image_surface_get_width(instrPanel->image_surface);
    int height = cairo_image_surface_get_height(instrPanel->image_surface);

    instrPanel->width = width + 20;
    instrPanel->height = height + 32;

    panel_init(&instrPanel->panel, renderer, instrPanel, instructions_panel_draw, panelManager, instrPanel->width, instrPanel->height);

    instructions_panel_add_titlebar_action_region(instrPanel);
    instructions_panel_add_close_button_action_region(instrPanel);
    instructions_panel_add_ok_button_action_region(instrPanel);

    instrPanel->ok_highlighted = false;

    return instrPanel;
}

void instructions_panel_destroy(InstructionsPanel* instrPanel) {
    cairo_surface_destroy(instrPanel->move_button_surface);
    cairo_surface_destroy(instrPanel->stamp_button_surface);
    cairo_surface_destroy(instrPanel->select_button_surface);
    cairo_surface_destroy(instrPanel->dropper_button_surface);
    cairo_surface_destroy(instrPanel->eraser_button_surface);
    cairo_surface_destroy(instrPanel->pencil_button_surface);
    cairo_surface_destroy(instrPanel->image_surface);
}

void instructions_panel_draw_background(InstructionsPanel* instrPanel) {
    cairo_set_source_rgb(instrPanel->panel.cr, 0.75, 0.75, 0.75);
    cairo_rectangle(instrPanel->panel.cr, 0, 0, instrPanel->width, instrPanel->height);
    cairo_fill(instrPanel->panel.cr);
    cairo_set_source_rgb(instrPanel->panel.cr, 0, 0, 0);
    cairo_rectangle(instrPanel->panel.cr, 0, 0, instrPanel->width, instrPanel->height);
    cairo_stroke(instrPanel->panel.cr);
}

void instructions_panel_draw_titlebar(InstructionsPanel* instrPanel) {
    cairo_set_source_rgb(instrPanel->panel.cr, 33.0f/255.0f, 0, 206.0f/255.0f);
    cairo_rectangle(instrPanel->panel.cr, 4, 4, instrPanel->width - 8, 16);
    cairo_fill(instrPanel->panel.cr);
}


void instructions_panel_draw_title_buttons(InstructionsPanel* instrPanel) {
    cairo_set_source_rgb(instrPanel->panel.cr, 0.75, 0.75, 0.75);
    cairo_rectangle(instrPanel->panel.cr, instrPanel->width - 18, 6, 12, 12);
    cairo_fill(instrPanel->panel.cr);
    cairo_set_source_rgb(instrPanel->panel.cr, 0, 0, 0);
    cairo_rectangle(instrPanel->panel.cr, instrPanel->width - 18, 6, 12, 12);
    cairo_stroke(instrPanel->panel.cr);

    int x = instrPanel->width - 18;
    int y = 6;
    int w = 12;
    int h = 12;
    cairo_move_to(instrPanel->panel.cr, x + 3, y + 3);
    cairo_line_to(instrPanel->panel.cr, x + w - 3, y + h - 3);
    cairo_move_to(instrPanel->panel.cr, x + w - 3, y + 3);
    cairo_line_to(instrPanel->panel.cr, x + 3, y + h - 3);
    cairo_stroke(instrPanel->panel.cr);
}

void instructions_panel_draw_image(InstructionsPanel* instrPanel) {
    cairo_set_source_surface(instrPanel->panel.cr, instrPanel->image_surface, 8, 24);
    cairo_paint(instrPanel->panel.cr);
}

void instructions_panel_draw_dialog_buttons(InstructionsPanel* instrPanel) {
    if (instrPanel->ok_highlighted) {
        cairo_set_source_rgb(instrPanel->panel.cr, 0, 0, 0);
        cairo_rectangle(instrPanel->panel.cr, instrPanel->width - 88, instrPanel->height - 36, 72, 24);
        cairo_fill(instrPanel->panel.cr);
        cairo_set_source_rgb(instrPanel->panel.cr, 1, 1, 1);
        cairo_rectangle(instrPanel->panel.cr, instrPanel->width - 88, instrPanel->height - 36, 72, 24);
        cairo_stroke(instrPanel->panel.cr);

        cairo_select_font_face(instrPanel->panel.cr, "Cantarell Regular", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(instrPanel->panel.cr, 12);

        cairo_move_to(instrPanel->panel.cr, instrPanel->width - 62, instrPanel->height - 20);
        cairo_show_text(instrPanel->panel.cr, "OK");
    } else {
        cairo_set_source_rgb(instrPanel->panel.cr, 0.88, 0.88, 0.88);
        cairo_rectangle(instrPanel->panel.cr, instrPanel->width - 88, instrPanel->height - 36, 72, 24);
        cairo_fill(instrPanel->panel.cr);
        cairo_set_source_rgb(instrPanel->panel.cr, 0, 0, 0);
        cairo_rectangle(instrPanel->panel.cr, instrPanel->width - 88, instrPanel->height - 36, 72, 24);
        cairo_stroke(instrPanel->panel.cr);

        cairo_select_font_face(instrPanel->panel.cr, "Cantarell Regular", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(instrPanel->panel.cr, 12);

        cairo_move_to(instrPanel->panel.cr, instrPanel->width - 62, instrPanel->height - 20);
        cairo_show_text(instrPanel->panel.cr, "OK");
    }
}

void instructions_panel_draw_pencil_button(InstructionsPanel* instrPanel) {
    cairo_surface_t* glyph = instrPanel->pencil_button_surface;

    cairo_set_source_surface(instrPanel->panel.cr, glyph, PENCIL_BUTTON_X, PENCIL_BUTTON_Y);
    cairo_paint(instrPanel->panel.cr);
}

void instructions_panel_draw_eraser_button(InstructionsPanel* instrPanel) {
    cairo_surface_t* glyph = instrPanel->eraser_button_surface;

    cairo_set_source_surface(instrPanel->panel.cr, glyph, ERASER_BUTTON_X, ERASER_BUTTON_Y);
    cairo_paint(instrPanel->panel.cr);
}

void instructions_panel_draw_dropper_button(InstructionsPanel* instrPanel) {
    cairo_surface_t* glyph = instrPanel->dropper_button_surface;

    cairo_set_source_surface(instrPanel->panel.cr, glyph, COLOR_DROPPER_BUTTON_X, COLOR_DROPPER_BUTTON_Y);
    cairo_paint(instrPanel->panel.cr);
}

void instructions_panel_draw_select_button(InstructionsPanel* instrPanel) {
    cairo_surface_t* glyph = instrPanel->select_button_surface;

    cairo_set_source_surface(instrPanel->panel.cr, glyph, SELECT_BUTTON_X, SELECT_BUTTON_Y);
    cairo_paint(instrPanel->panel.cr);
}

void instructions_panel_draw_stamp_button(InstructionsPanel* instrPanel) {
    cairo_surface_t* glyph = instrPanel->stamp_button_surface;

    cairo_set_source_surface(instrPanel->panel.cr, glyph, STAMP_BUTTON_X, STAMP_BUTTON_Y);
    cairo_paint(instrPanel->panel.cr);
}

void instructions_panel_draw_move_button(InstructionsPanel* instrPanel) {
    cairo_surface_t* glyph = instrPanel->move_button_surface;

    cairo_set_source_surface(instrPanel->panel.cr, glyph, MOVE_BUTTON_X, MOVE_BUTTON_Y);
    cairo_paint(instrPanel->panel.cr);
}

void instructions_panel_draw_buttons(InstructionsPanel* instrPanel) {
    instructions_panel_draw_pencil_button(instrPanel);
    instructions_panel_draw_eraser_button(instrPanel);
    instructions_panel_draw_dropper_button(instrPanel);
    instructions_panel_draw_select_button(instrPanel);
    instructions_panel_draw_stamp_button(instrPanel);
    instructions_panel_draw_move_button(instrPanel);
}

void instructions_panel_draw(void* instrPanelPtr) {
    InstructionsPanel* instrPanel = (InstructionsPanel*)instrPanelPtr;

    instructions_panel_draw_background(instrPanel);
    instructions_panel_draw_titlebar(instrPanel);
    instructions_panel_draw_title_buttons(instrPanel);
    instructions_panel_draw_image(instrPanel);
    instructions_panel_draw_buttons(instrPanel);
    instructions_panel_draw_dialog_buttons(instrPanel);
}

void instructions_panel_add_titlebar_action_region(InstructionsPanel* instrPanel) {
    ActionRegion* actionRegion = NEW(ActionRegion, 1);
    actionRegion->position[0] = 0;
    actionRegion->position[1] = 0;
    actionRegion->width = instrPanel->width - 20;
    actionRegion->height = 24;
    actionRegion->action_press = instructions_panel_titlebar_press;
    actionRegion->action_release = instructions_panel_titlebar_release;
    panel_add_action_region(&instrPanel->panel, actionRegion);
}

void instructions_panel_add_close_button_action_region(InstructionsPanel* instrPanel) {
    ActionRegion* actionRegion = NEW(ActionRegion, 1);
    actionRegion->position[0] = instrPanel->width - 18;
    actionRegion->position[1] = 6;
    actionRegion->width = 12;
    actionRegion->height = 12;
    actionRegion->action_press = NULL;
    actionRegion->action_release = instructions_panel_close_button_release;
    panel_add_action_region(&instrPanel->panel, actionRegion);
}

void instructions_panel_add_ok_button_action_region(InstructionsPanel* instrPanel) {
    ActionRegion* actionRegion = NEW(ActionRegion, 1);
    actionRegion->position[0] = instrPanel->width - 88;
    actionRegion->position[1] = instrPanel->height - 36;
    actionRegion->width = 72;
    actionRegion->height = 24;
    actionRegion->action_press = instructions_panel_ok_button_press;
    actionRegion->action_release = instructions_panel_ok_button_release;
    panel_add_action_region(&instrPanel->panel, actionRegion);
}

void instructions_panel_set_position(InstructionsPanel* instrPanel, unsigned int x, unsigned int y) {
    panel_set_position(&instrPanel->panel, x, y);
}