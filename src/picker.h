#ifndef PICKER_H
#define PICKER_H

#include "global.h"
#include "box.h"
#include "camera.h"
#include "world.h"

#define   PICKER_ONTO           1
#define   PICKER_ADJACENT       2

#define   PICKER_CLEAR          3
#define   PICKER_SET            4

typedef struct {
    World* world;

    int positionStart[3];
    int positionEnd[3];
    
    Box box;
    Mesh mesh;

    char dragging;
    char mode;
    char action;

    uint16_t color;
} Picker;

void picker_init(Picker* p);
void picker_destroy(Picker* picker);
void picker_update(Picker* picker, Camera* camera, GLfloat mouseX, GLfloat mouseY);

void picker_press(Picker* picker, char modifier1, char modifier2);
void picker_release(Picker* picker, char modifier1, char modifier2);

void picker_set_action(Picker* picker, char action);

void picker_set_world(Picker* picker, World* world);

#endif // PICKER_H
