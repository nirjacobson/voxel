#ifndef PICKER_H
#define PICKER_H

#include <glib.h>

#include "global.h"
#include "box.h"
#include "camera.h"
#include "world.h"
#include "undo_stack.h"
#include "commands/world_clear_region_command.h"
#include "commands/world_copy_chunk_command.h"
#include "commands/world_cut_chunk_command.h"
#include "commands/world_set_region_command.h"

#define PICKER_ONTO        1
#define PICKER_ADJACENT    2

#define PICKER_CLEAR       3
#define PICKER_SET         4
#define PICKER_EYEDROPPER  5
#define PICKER_SELECT      6
#define PICKER_STAMP       7
#define PICKER_MOVE        8

typedef struct {
    Box box;
    Mesh mesh;
    Chunk* model;
    char rotation;
    char present;
} Selection;

typedef struct {
    World* world;
    UndoStack* undoStack;

    int positionStart[3];
    int positionEnd[3];
    Box box;
    Mesh mesh;
    Selection selection;

    char dragging;
    char mode;
    char action;

    uint16_t color;
} Picker;

void picker_init(Picker* p, World* world, UndoStack* undoStack);
void picker_destroy(Picker* picker);
void picker_update(Picker* picker, Camera* camera, float mouseX, float mouseY);

void picker_press(Picker* picker, char modifier1, char modifier2);
void picker_release(Picker* picker, char modifier1, char modifier2);

void picker_set_action(Picker* picker, char action);

#endif // PICKER_H
