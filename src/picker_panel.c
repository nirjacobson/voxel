#include "picker_panel.h"
#include "internal/picker_panel.h"

#include "voxel.h"

/* Action region callbacks */

void picker_panel_titlebar_press(ActionRegionArgs* args) {
    args->panel->manager->dragging = 1;
}

void picker_panel_titlebar_release(ActionRegionArgs* args) {
    args->panel->manager->dragging = 0;
}

void picker_panel_bluebar_press(ActionRegionArgs* args) {
    PickerPanel* pickerPanel = (PickerPanel*)args->panel->owner;

    BLOCK_COLOR_SET_BLUE(pickerPanel->picker->color, args->x / 15);
}

void picker_panel_palette_press(ActionRegionArgs* args) {
    PickerPanel* pickerPanel = (PickerPanel*)args->panel->owner;

    BLOCK_COLOR_SET_RED(pickerPanel->picker->color, 7 - (args->y / 15));
    BLOCK_COLOR_SET_GREEN(pickerPanel->picker->color, args->x / 15);
}

void picker_panel_pencil_button_press(ActionRegionArgs* args) {
    PickerPanel* pickerPanel = (PickerPanel*)args->panel->owner;
    picker_set_action(pickerPanel->picker, PICKER_SET);
}

void picker_panel_eraser_button_press(ActionRegionArgs* args) {
    PickerPanel* pickerPanel = (PickerPanel*)args->panel->owner;
    picker_set_action(pickerPanel->picker, PICKER_CLEAR);
}

void picker_panel_dropper_button_press(ActionRegionArgs* args) {
    PickerPanel* pickerPanel = (PickerPanel*)args->panel->owner;
    picker_set_action(pickerPanel->picker, PICKER_EYEDROPPER);
}

void picker_panel_select_button_press(ActionRegionArgs* args) {
    PickerPanel* pickerPanel = (PickerPanel*)args->panel->owner;
    picker_set_action(pickerPanel->picker, PICKER_SELECT);
}

void picker_panel_stamp_button_press(ActionRegionArgs* args) {
    PickerPanel* pickerPanel = (PickerPanel*)args->panel->owner;
    picker_set_action(pickerPanel->picker, PICKER_STAMP);
}

void picker_panel_move_button_press(ActionRegionArgs* args) {
    PickerPanel* pickerPanel = (PickerPanel*)args->panel->owner;
    picker_set_action(pickerPanel->picker, PICKER_MOVE);
}

void picker_panel_question_button_press(ActionRegionArgs* args) {
    PickerPanel* pickerPanel = (PickerPanel*)args->panel->owner;
    pickerPanel->question_highlighted = true;
    picker_panel_draw_question_button(pickerPanel);
}

void picker_panel_question_button_release(ActionRegionArgs* args) {
    PickerPanel* pickerPanel = (PickerPanel*)args->panel->owner;
    pickerPanel->question_highlighted = false;
    picker_panel_draw_question_button(pickerPanel);

    voxel_show_instructions_panel(pickerPanel->voxel);
}

/* PickerPanel */

cairo_status_t cairo_read_func(void* closure, unsigned char* data, unsigned int length) {
    unsigned int read_len = g_input_stream_read((GInputStream*)closure, data, length, (void*)NULL, (GError**)NULL);

    if (read_len == length) {
        return CAIRO_STATUS_SUCCESS;
    }

    return CAIRO_STATUS_READ_ERROR;
}

PickerPanel* picker_panel_init(PickerPanel* pp, Voxel* voxel, Renderer* renderer, PanelManager* panelManager, Picker* picker) {
    PickerPanel* pickerPanel = pp ? pp : NEW(PickerPanel, 1);

    panel_init(&pickerPanel->panel, renderer, pickerPanel, picker_panel_draw, panelManager, PICKER_PANEL_WIDTH, PICKER_PANEL_HEIGHT);

    GInputStream* inputStream;
    
    const char* paths[] = {
        "/img/pencil-natural.png",
        "/img/pencil-selected.png",
        "/img/eraser-natural.png",
        "/img/eraser-selected.png",
        "/img/color-dropper-natural.png",
        "/img/color-dropper-selected.png",
        "/img/select-natural.png",
        "/img/select-selected.png",
        "/img/stamp-natural.png",
        "/img/stamp-selected.png",
        "/img/move-natural.png",
        "/img/move-selected.png"
    };

    cairo_surface_t** surfaces[] = {
        &pickerPanel->pencil_button_surface_natural,
        &pickerPanel->pencil_button_surface_selected,
        &pickerPanel->eraser_button_surface_natural,
        &pickerPanel->eraser_button_surface_selected,
        &pickerPanel->dropper_button_surface_natural,
        &pickerPanel->dropper_button_surface_selected,
        &pickerPanel->select_button_surface_natural,
        &pickerPanel->select_button_surface_selected,
        &pickerPanel->stamp_button_surface_natural,
        &pickerPanel->stamp_button_surface_selected,
        &pickerPanel->move_button_surface_natural,
        &pickerPanel->move_button_surface_selected
    };

    int count = sizeof(surfaces) / sizeof(surfaces[0]);

    for (int i = 0; i < count; i++) {
        inputStream = g_resources_open_stream(paths[i], G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
        *surfaces[i] = cairo_image_surface_create_from_png_stream(cairo_read_func, (void*)inputStream);
    }

    pickerPanel->picker = picker;
    pickerPanel->voxel = voxel;

    pickerPanel->picker->color = 0;

    picker_panel_add_titlebar_action_region(pickerPanel);
    picker_panel_add_bluebar_action_region(pickerPanel);
    picker_panel_add_palette_action_region(pickerPanel);
    picker_panel_add_pencil_button_action_region(pickerPanel);
    picker_panel_add_eraser_button_action_region(pickerPanel);
    picker_panel_add_dropper_button_action_region(pickerPanel);
    picker_panel_add_select_button_action_region(pickerPanel);
    picker_panel_add_stamp_button_action_region(pickerPanel);
    picker_panel_add_move_button_action_region(pickerPanel);
    picker_panel_add_question_button_action_region(pickerPanel);

    pickerPanel->question_highlighted = false;

    return pickerPanel;
}

void picker_panel_destroy(PickerPanel* pickerPanel) {
    cairo_surface_destroy(pickerPanel->move_button_surface_selected);
    cairo_surface_destroy(pickerPanel->move_button_surface_natural);
    cairo_surface_destroy(pickerPanel->stamp_button_surface_selected);
    cairo_surface_destroy(pickerPanel->stamp_button_surface_natural);
    cairo_surface_destroy(pickerPanel->select_button_surface_selected);
    cairo_surface_destroy(pickerPanel->select_button_surface_natural);
    cairo_surface_destroy(pickerPanel->dropper_button_surface_selected);
    cairo_surface_destroy(pickerPanel->dropper_button_surface_natural);
    cairo_surface_destroy(pickerPanel->eraser_button_surface_selected);
    cairo_surface_destroy(pickerPanel->eraser_button_surface_natural);
    cairo_surface_destroy(pickerPanel->pencil_button_surface_selected);
    cairo_surface_destroy(pickerPanel->pencil_button_surface_natural);
    panel_destroy(&pickerPanel->panel);
}

void picker_panel_draw_background(PickerPanel* pickerPanel) {
    cairo_set_source_rgb(pickerPanel->panel.cr, 0.75, 0.75, 0.75);
    cairo_rectangle(pickerPanel->panel.cr, 0, 0, PICKER_PANEL_WIDTH, PICKER_PANEL_HEIGHT);
    cairo_fill(pickerPanel->panel.cr);
    cairo_set_source_rgb(pickerPanel->panel.cr, 0, 0, 0);
    cairo_rectangle(pickerPanel->panel.cr, 0, 0, PICKER_PANEL_WIDTH, PICKER_PANEL_HEIGHT);
    cairo_stroke(pickerPanel->panel.cr);
}

void picker_panel_draw_titlebar(PickerPanel* pickerPanel) {
    cairo_set_source_rgb(pickerPanel->panel.cr, 33.0f/255.0f, 0, 206.0f/255.0f);
    cairo_rectangle(pickerPanel->panel.cr, 4, 4, PICKER_PANEL_WIDTH - 8, 16);
    cairo_fill(pickerPanel->panel.cr);
}

void picker_panel_draw_bluebar(PickerPanel* pickerPanel) {
    GLuint x, y, w, h;
    for (int i = 0; i < 8; i++) {
        x = BLUE_BAR_X + (i * 15);
        y = BLUE_BAR_Y;
        w = 16;
        h = 16;

        cairo_set_source_rgb(pickerPanel->panel.cr, 0, 0, 0);
        cairo_rectangle(pickerPanel->panel.cr, x, y, w, h);
        cairo_fill(pickerPanel->panel.cr);

        x++;
        y++;
        w = 14;
        h = 14;
        cairo_set_source_rgb(pickerPanel->panel.cr, 0.75, 0.75, 0.75);
        cairo_rectangle(pickerPanel->panel.cr, x, y, w, h);
        cairo_fill(pickerPanel->panel.cr);
        cairo_set_source_rgba(pickerPanel->panel.cr, 0, 0, 1, 1.0 / 7 * i);
        cairo_rectangle(pickerPanel->panel.cr, x, y, w, h);
        cairo_fill(pickerPanel->panel.cr);

        if (i == BLOCK_COLOR_BLUE(pickerPanel->picker->color)) {
            float average = 255.0/7 * i / 3;
            if (average <= 255.0/3) {
                cairo_set_source_rgb(pickerPanel->panel.cr, 1, 1, 1);
            } else {
                cairo_set_source_rgb(pickerPanel->panel.cr, 0, 0, 0);
            }
            cairo_move_to(pickerPanel->panel.cr, x, y + 1);
            cairo_line_to(pickerPanel->panel.cr, x + w, y + h - 1);
            cairo_move_to(pickerPanel->panel.cr, x + w, y + 1);
            cairo_line_to(pickerPanel->panel.cr, x, y + h - 1);
            cairo_stroke(pickerPanel->panel.cr);
        }
    }
}

void picker_panel_draw_palette(PickerPanel* pickerPanel) {
    GLuint rx, ry, rw, rh;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            rx = PALETTE_X + (x * 15);
            ry = PALETTE_Y + (y * 15);
            rw = 16;
            rh = 16;
            cairo_set_source_rgb(pickerPanel->panel.cr, 0, 0, 0);
            cairo_rectangle(pickerPanel->panel.cr, rx, ry, rw, rh);
            cairo_fill(pickerPanel->panel.cr);

            rx++;
            ry++;
            rw = 14;
            rh = 14;
            cairo_set_source_rgb(pickerPanel->panel.cr, (7 - y) * (1.0/7), x * (1.0/7), BLOCK_COLOR_BLUE(pickerPanel->picker->color) * (1.0/7));
            cairo_rectangle(pickerPanel->panel.cr, rx, ry, rw, rh);
            cairo_fill(pickerPanel->panel.cr);


            if (x == BLOCK_COLOR_GREEN(pickerPanel->picker->color) && (7 - y) == BLOCK_COLOR_RED(pickerPanel->picker->color)) {
                float average = ((7-y) * (255.0/7) + x * (255.0/7) + BLOCK_COLOR_BLUE(pickerPanel->picker->color) * (255.0/7)) / 3;
                if (average <= 255.0/3) {
                    cairo_set_source_rgb(pickerPanel->panel.cr, 1, 1, 1);
                } else {
                    cairo_set_source_rgb(pickerPanel->panel.cr, 0, 0, 0);
                }
                cairo_move_to(pickerPanel->panel.cr, rx, ry + 1);
                cairo_line_to(pickerPanel->panel.cr, rx + rw, ry + rh - 1);
                cairo_move_to(pickerPanel->panel.cr, rx + rw, ry + 1);
                cairo_line_to(pickerPanel->panel.cr, rx, ry + rh - 1);
                cairo_stroke(pickerPanel->panel.cr);
            }
        }
    }
}

void picker_panel_draw_pencil_button(PickerPanel* pickerPanel) {
    cairo_surface_t* glyph =
        pickerPanel->picker->action == PICKER_SET
        ? pickerPanel->pencil_button_surface_selected
        : pickerPanel->pencil_button_surface_natural;

    cairo_set_source_surface(pickerPanel->panel.cr, glyph, PENCIL_BUTTON_X, PENCIL_BUTTON_Y);
    cairo_paint(pickerPanel->panel.cr);
}

void picker_panel_draw_eraser_button(PickerPanel* pickerPanel) {
    cairo_surface_t* glyph =
        pickerPanel->picker->action == PICKER_CLEAR
        ? pickerPanel->eraser_button_surface_selected
        : pickerPanel->eraser_button_surface_natural;

    cairo_set_source_surface(pickerPanel->panel.cr, glyph, ERASER_BUTTON_X, ERASER_BUTTON_Y);
    cairo_paint(pickerPanel->panel.cr);
}

void picker_panel_draw_dropper_button(PickerPanel* pickerPanel) {
    cairo_surface_t* glyph =
        pickerPanel->picker->action == PICKER_EYEDROPPER
        ? pickerPanel->dropper_button_surface_selected
        : pickerPanel->dropper_button_surface_natural;

    cairo_set_source_surface(pickerPanel->panel.cr, glyph, COLOR_DROPPER_BUTTON_X, COLOR_DROPPER_BUTTON_Y);
    cairo_paint(pickerPanel->panel.cr);
}

void picker_panel_draw_select_button(PickerPanel* pickerPanel) {
    cairo_surface_t* glyph =
        pickerPanel->picker->action == PICKER_SELECT
        ? pickerPanel->select_button_surface_selected
        : pickerPanel->select_button_surface_natural;

    cairo_set_source_surface(pickerPanel->panel.cr, glyph, SELECT_BUTTON_X, SELECT_BUTTON_Y);
    cairo_paint(pickerPanel->panel.cr);
}

void picker_panel_draw_stamp_button(PickerPanel* pickerPanel) {
    if (pickerPanel->picker->selection.present) {
        cairo_surface_t* glyph =
            pickerPanel->picker->action == PICKER_STAMP
            ? pickerPanel->stamp_button_surface_selected
            : pickerPanel->stamp_button_surface_natural;

        cairo_set_source_surface(pickerPanel->panel.cr, glyph, STAMP_BUTTON_X, STAMP_BUTTON_Y);
        cairo_paint(pickerPanel->panel.cr);
    }
}

void picker_panel_draw_move_button(PickerPanel* pickerPanel) {
    if (pickerPanel->picker->selection.present) {
        cairo_surface_t* glyph =
            pickerPanel->picker->action == PICKER_MOVE
            ? pickerPanel->move_button_surface_selected
            : pickerPanel->move_button_surface_natural;

        cairo_set_source_surface(pickerPanel->panel.cr, glyph, MOVE_BUTTON_X, MOVE_BUTTON_Y);
        cairo_paint(pickerPanel->panel.cr);
    }
}

void picker_panel_draw_buttons(PickerPanel* pickerPanel) {
    picker_panel_draw_pencil_button(pickerPanel);
    picker_panel_draw_eraser_button(pickerPanel);
    picker_panel_draw_dropper_button(pickerPanel);
    picker_panel_draw_select_button(pickerPanel);
    picker_panel_draw_stamp_button(pickerPanel);
    picker_panel_draw_move_button(pickerPanel);
}

void picker_panel_draw_question_button(PickerPanel* pickerPanel) {
    if (pickerPanel->question_highlighted) {
        cairo_set_source_rgb(pickerPanel->panel.cr, 0, 0, 0);
        cairo_set_line_width(pickerPanel->panel.cr, 0.75);
        cairo_arc(pickerPanel->panel.cr, PALETTE_X - 24 + 8, PICKER_PANEL_HEIGHT - 24 + 8, 8, 0, 2 * M_PI);
        cairo_fill(pickerPanel->panel.cr);
        cairo_set_source_rgb(pickerPanel->panel.cr, 1, 1, 1);
        cairo_arc(pickerPanel->panel.cr, PALETTE_X - 24 + 8, PICKER_PANEL_HEIGHT - 24 + 8, 8, 0, 2 * M_PI);
        cairo_stroke(pickerPanel->panel.cr);

        cairo_select_font_face(pickerPanel->panel.cr, "Cantarell Regular", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(pickerPanel->panel.cr, 12);

        cairo_move_to(pickerPanel->panel.cr, PALETTE_X - 19, PICKER_PANEL_HEIGHT - 11);
        cairo_show_text(pickerPanel->panel.cr, "?");
    } else {
        cairo_set_source_rgb(pickerPanel->panel.cr, 0, 0, 0);
        cairo_set_line_width(pickerPanel->panel.cr, 0.75);
        cairo_arc(pickerPanel->panel.cr, PALETTE_X - 24 + 8, PICKER_PANEL_HEIGHT - 24 + 8, 8, 0, 2 * M_PI);
        cairo_stroke(pickerPanel->panel.cr);

        cairo_select_font_face(pickerPanel->panel.cr, "Cantarell Regular", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(pickerPanel->panel.cr, 12);

        cairo_move_to(pickerPanel->panel.cr, PALETTE_X - 19, PICKER_PANEL_HEIGHT - 11);
        cairo_show_text(pickerPanel->panel.cr, "?");
    }
}

void picker_panel_draw(void* pickerPanelPtr) {
    PickerPanel* pickerPanel = (PickerPanel*)pickerPanelPtr;

    picker_panel_draw_background(pickerPanel);
    picker_panel_draw_titlebar(pickerPanel);
    picker_panel_draw_bluebar(pickerPanel);
    picker_panel_draw_palette(pickerPanel);
    picker_panel_draw_buttons(pickerPanel);
    picker_panel_draw_question_button(pickerPanel);
}

void picker_panel_add_titlebar_action_region(PickerPanel* pickerPanel) {
    ActionRegion* actionRegion = NEW(ActionRegion, 1);
    actionRegion->position[0] = 0;
    actionRegion->position[1] = 0;
    actionRegion->width = PICKER_PANEL_WIDTH;
    actionRegion->height = 16;
    actionRegion->action_press = picker_panel_titlebar_press;
    actionRegion->action_release = picker_panel_titlebar_release;
    panel_add_action_region(&pickerPanel->panel, actionRegion);
}

void picker_panel_add_bluebar_action_region(PickerPanel* pickerPanel) {
    ActionRegion* actionRegion = NEW(ActionRegion, 1);

    actionRegion->position[0] = BLUE_BAR_X;
    actionRegion->position[1] = BLUE_BAR_Y;
    actionRegion->width = 8 * 15 + 1;
    actionRegion->height = 16;
    actionRegion->action_press = picker_panel_bluebar_press;
    actionRegion->action_release = NULL;
    panel_add_action_region(&pickerPanel->panel, actionRegion);
}

void picker_panel_add_palette_action_region(PickerPanel* pickerPanel) {
    ActionRegion* actionRegion = NEW(ActionRegion, 1);

    actionRegion->position[0] = PALETTE_X;
    actionRegion->position[1] = PALETTE_Y;
    actionRegion->width = 8 * 15 + 1;
    actionRegion->height = 8 * 15 + 1;
    actionRegion->action_press = picker_panel_palette_press;
    actionRegion->action_release = NULL;
    panel_add_action_region(&pickerPanel->panel, actionRegion);
}

void picker_panel_add_pencil_button_action_region(PickerPanel* pickerPanel) {
    ActionRegion* actionRegion = NEW(ActionRegion, 1);

    actionRegion->position[0] = PENCIL_BUTTON_X;
    actionRegion->position[1] = PENCIL_BUTTON_Y;
    actionRegion->width = BUTTON_WIDTH;
    actionRegion->height = BUTTON_HEIGHT;
    actionRegion->action_press = picker_panel_pencil_button_press;
    actionRegion->action_release = NULL;
    panel_add_action_region(&pickerPanel->panel, actionRegion);
}

void picker_panel_add_eraser_button_action_region(PickerPanel* pickerPanel) {
    ActionRegion* actionRegion = NEW(ActionRegion, 1);

    actionRegion->position[0] = ERASER_BUTTON_X;
    actionRegion->position[1] = ERASER_BUTTON_Y;
    actionRegion->width = BUTTON_WIDTH;
    actionRegion->height = BUTTON_HEIGHT;
    actionRegion->action_press = picker_panel_eraser_button_press;
    actionRegion->action_release = NULL;
    panel_add_action_region(&pickerPanel->panel, actionRegion);
}

void picker_panel_add_dropper_button_action_region(PickerPanel* pickerPanel) {
    ActionRegion* actionRegion = NEW(ActionRegion, 1);

    actionRegion->position[0] = COLOR_DROPPER_BUTTON_X;
    actionRegion->position[1] = COLOR_DROPPER_BUTTON_Y;
    actionRegion->width = BUTTON_WIDTH;
    actionRegion->height = BUTTON_HEIGHT;
    actionRegion->action_press = picker_panel_dropper_button_press;
    actionRegion->action_release = NULL;
    panel_add_action_region(&pickerPanel->panel, actionRegion);
}

void picker_panel_add_select_button_action_region(PickerPanel* pickerPanel) {
    ActionRegion* actionRegion = NEW(ActionRegion, 1);

    actionRegion->position[0] = SELECT_BUTTON_X;
    actionRegion->position[1] = SELECT_BUTTON_Y;
    actionRegion->width = BUTTON_WIDTH;
    actionRegion->height = BUTTON_HEIGHT;
    actionRegion->action_press = picker_panel_select_button_press;
    actionRegion->action_release = NULL;
    panel_add_action_region(&pickerPanel->panel, actionRegion);
}

void picker_panel_add_stamp_button_action_region(PickerPanel* pickerPanel) {
    ActionRegion* actionRegion = NEW(ActionRegion, 1);

    actionRegion->position[0] = STAMP_BUTTON_X;
    actionRegion->position[1] = STAMP_BUTTON_Y;
    actionRegion->width = BUTTON_WIDTH;
    actionRegion->height = BUTTON_HEIGHT;
    actionRegion->action_press = picker_panel_stamp_button_press;
    actionRegion->action_release = NULL;
    panel_add_action_region(&pickerPanel->panel, actionRegion);
}

void picker_panel_add_move_button_action_region(PickerPanel* pickerPanel) {
    ActionRegion* actionRegion = NEW(ActionRegion, 1);

    actionRegion->position[0] = MOVE_BUTTON_X;
    actionRegion->position[1] = MOVE_BUTTON_Y;
    actionRegion->width = BUTTON_WIDTH;
    actionRegion->height = BUTTON_HEIGHT;
    actionRegion->action_press = picker_panel_move_button_press;
    actionRegion->action_release = NULL;
    panel_add_action_region(&pickerPanel->panel, actionRegion);
}

void picker_panel_add_question_button_action_region(PickerPanel* pickerPanel) {
    ActionRegion* actionRegion = NEW(ActionRegion, 1);

    actionRegion->position[0] = PALETTE_X - 24;
    actionRegion->position[1] = PICKER_PANEL_HEIGHT - 24;
    actionRegion->width = 16;
    actionRegion->height = 16;
    actionRegion->action_press = picker_panel_question_button_press;
    actionRegion->action_release = picker_panel_question_button_release;
    panel_add_action_region(&pickerPanel->panel, actionRegion);
}