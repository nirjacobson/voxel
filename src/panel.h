#ifndef PANEL_H
#define PANEL_H

#include <cairo/cairo.h>
#include <stdlib.h>

#include "GLES2/gl2.h"

#include "global.h"
#include "linked_list.h"
#include "renderer.h"
#include "mouse.h"

struct Panel;
struct PanelManager;
struct ActionRegion;

/* ActionRegion */

struct ActionRegionArgs {
  void* owner;
  struct Panel* panel;
  struct ActionRegion* actionRegion;
  unsigned int x;
  unsigned int y;
};

typedef struct ActionRegionArgs ActionRegionArgs;

struct ActionRegion {
  unsigned int position[2];
  unsigned int width;
  unsigned int height;

  void (*action_press)(struct ActionRegionArgs*);
  void (*action_release)(struct ActionRegionArgs*);
};

typedef struct ActionRegion ActionRegion;

/* Panel */

struct Panel {
  unsigned int vbo;
  unsigned int tex;

  cairo_surface_t *surface;
  cairo_t *cr;

  void* owner;
  void (*drawCallback)(void*);
  struct PanelManager* manager;

  unsigned int position[2];
  unsigned int width;
  unsigned int height;

  LinkedList actionRegions;
};

typedef struct Panel Panel;

Panel* panel_init(Panel* d, void* owner, void (*drawCallback)(void*), struct PanelManager* manager, unsigned int width, unsigned int height);
void panel_destroy(Panel* panel);

void panel_add_action_region(Panel* panel, ActionRegion* actionRegion);
void panel_action(Panel* panel, char action, unsigned int x, unsigned int y);

void panel_set_position(Panel* panel, int x, int y);
void panel_translate(Panel* panel, int x, int y);
void panel_texture(Panel* panel);

void panel_draw(Panel* panel);

/* PanelManager */

struct PanelManager{
  Renderer* renderer;

  LinkedList panels;
  Panel* active_panel;
  char dragging;
};

typedef struct PanelManager PanelManager;

PanelManager* panel_manager_init(PanelManager* pm, Renderer* renderer);
void panel_manager_destroy(PanelManager* panelManager);
void panel_manager_add_panel(PanelManager* panelManager, Panel* panel);
Panel* panel_manager_find_panel(PanelManager* panelManager, unsigned int x, unsigned int y);

void panel_manager_draw(PanelManager* panelManager);

#endif // PANEL_H
