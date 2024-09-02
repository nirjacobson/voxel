#ifndef VOXEL_H
#define VOXEL_H

#include <sys/time.h>
#include <gio/gio.h>

#include "global.h"
#include "application.h"
#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "world.h"
#include "picker.h"
#include "panel.h"
#include "fps_panel.h"
#include "picker_panel.h"
#include "undo_stack.h"
#include "vulkan_util.h"

typedef struct {
    Application application;
    Window window;

    Vulkan* vulkan;

    Renderer renderer;

    World world;
    Camera camera;
    Picker picker;

    PanelManager panelManager;

    FPSPanel fpsPanel;
    PickerPanel pickerPanel;

    UndoStack undoStack;

    struct timeval frameTime;
} Voxel;

Voxel* voxel_init(Voxel* v);
void voxel_destroy(Voxel* voxel);

void voxel_run(Voxel* voxel);

#endif // VOXEL_H
