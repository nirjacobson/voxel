#ifndef PANEL_H
#define PANEL_H

#include <cairo/cairo.h>
#include <stdlib.h>

#include "global.h"
#include "linked_list.h"
#include "window.h"
#include "vulkan_util.h"
#include "renderer.h"

const extern int MAX_FRAMES_IN_FLIGHT;

struct Panel;
struct PanelManager;
struct ActionRegion;

/* ActionRegion */

typedef struct {
    struct Panel* panel;
    struct ActionRegion* actionRegion;
    unsigned int x;
    unsigned int y;
} ActionRegionArgs;

typedef struct ActionRegion {
    unsigned int position[2];
    unsigned int width;
    unsigned int height;

    void (*action_press)(ActionRegionArgs*);
    void (*action_release)(ActionRegionArgs*);
} ActionRegion;

/* Panel */

typedef struct Panel {
    Vulkan* vulkan;
    Renderer* renderer;

    union {
        struct {
            unsigned int vbo;
            unsigned int tex;
        } opengl;
        struct {
            VkBuffer vbo;
            VkDeviceMemory vboDeviceMemory;
            VkImage texImage;
            VkDeviceMemory texImageDeviceMemory;
            VkImageView texImageView;
            VkDescriptorSet* descriptorSets;
        } vulkan;
    } renderState;

    cairo_surface_t *surface;
    cairo_t *cr;

    void* owner;
    void (*drawCallback)(void*);
    struct PanelManager* manager;

    unsigned int position[2];
    unsigned int width;
    unsigned int height;

    LinkedList actionRegions;
} Panel;

Panel* panel_init(Panel* d, Renderer* renderer, void* owner, void (*drawCallback)(void*), struct PanelManager* manager, unsigned int width, unsigned int height);
void panel_destroy(Panel* panel);

void panel_add_action_region(Panel* panel, ActionRegion* actionRegion);
void panel_action(Panel* panel, char action, unsigned int x, unsigned int y);

void panel_set_position(Panel* panel, int x, int y);
void panel_translate(Panel* panel, int x, int y);

void panel_texture(Panel* panel);

/* PanelManager */

typedef struct PanelManager {
    LinkedList panels;
    Panel* active_panel;
    char dragging;
} PanelManager;

PanelManager* panel_manager_init(PanelManager* pm);
void panel_manager_destroy(PanelManager* panelManager);
void panel_manager_add_panel(PanelManager* panelManager, Panel* panel);
Panel* panel_manager_find_panel(PanelManager* panelManager, unsigned int x, unsigned int y);

#endif // PANEL_H
