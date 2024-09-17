#ifndef VOXEL_H
#define VOXEL_H

#ifdef _WIN32
#include <winsock.h>
#endif

#ifndef _WIN32
#include <sys/time.h>
#endif

#include <gio/gio.h>

#include "global.h"
#include "vulkan_util.h"
#include "application.h"
#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "world.h"
#include "picker.h"
#include "panel.h"
#include "fps_panel.h"
#include "picker_panel.h"
#include "instructions_panel.h"
#include "undo_stack.h"

typedef struct Voxel {
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
    InstructionsPanel instrPanel;

    UndoStack undoStack;

    struct timeval frameTime;
} Voxel;

Voxel* voxel_init(Voxel* v);
void voxel_destroy(Voxel* voxel);

void voxel_show_instructions_panel(Voxel* voxel);

void voxel_run(Voxel* voxel);

#endif // VOXEL_H
