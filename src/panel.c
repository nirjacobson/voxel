#include "panel.h"

/* Linked list processing callbacks */

char coords_over_action_region(void* coordsPtr, void* actionRegionPtr) {
  unsigned int* coords = (unsigned int*)coordsPtr;
  ActionRegion* actionRegion = (ActionRegion*)actionRegionPtr;

  return (coords[0] >= actionRegion->position[0]) && (coords[0] < (actionRegion->position[0] + actionRegion->width)) &&
         (coords[1] >= actionRegion->position[1]) && (coords[1] < (actionRegion->position[1] + actionRegion->height));
}

char coords_over_panel(void* coordsPtr, void* panelPtr) {
  unsigned int* coords = (unsigned int*)coordsPtr;
  Panel* panel = (Panel*)panelPtr;

  return (coords[0] >= panel->position[0]) && (coords[0] < (panel->position[0] + panel->width)) &&
         (coords[1] >= panel->position[1]) && (coords[1] < (panel->position[1] + panel->height));
}

void draw_panel(void* panelPtr, void* unusued) {
  Panel* panel = (Panel*)panelPtr;
  panel_draw(panel);
}

/* Panel */

Panel* panel_init(Panel* d, void* owner, void (*drawCallback)(void*), PanelManager* manager, unsigned int width, unsigned int height) {
  Panel* panel = d ? d : NEW(Panel, 1);

  panel->position[0] = 10;
  panel->position[1] = 10;

  panel->width = width;
  panel->height = height;

  panel->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, panel->width, panel->height);
  panel->cr = cairo_create (panel->surface);

  panel->owner = owner;
  panel->drawCallback = drawCallback;
  panel->manager = manager;

  linked_list_init(&panel->actionRegions);

  glGenBuffers(1, &panel->vbo);
  glGenTextures(1, &panel->tex);

  GLfloat vertex_data[] =  {
    panel->position[0], panel->position[1], -0.5, 0, 0,
    panel->position[0], panel->position[1] + panel->height, -0.5, 0, 1,
    panel->position[0] + panel->width, panel->position[1], -0.5, 1, 0,
    panel->position[0] + panel->width, panel->position[1] + panel->height, -0.5, 1, 1
  };
  glBindBuffer(GL_ARRAY_BUFFER, panel->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

  panel_manager_add_panel(panel->manager, panel);

  return panel;
}

void panel_destroy(Panel* panel) {
  glDeleteTextures(1, &panel->tex);
  glDeleteBuffers(1, &panel->vbo);

  linked_list_destroy(&panel->actionRegions, free);

  cairo_destroy(panel->cr);
  cairo_surface_destroy(panel->surface);
}

void panel_add_action_region(Panel* panel, ActionRegion* actionRegion) {
  linked_list_insert(&panel->actionRegions, actionRegion);
}

void panel_action(Panel* panel, char action, unsigned int x, unsigned int y) {
  if (action == MOUSE_PRESS) {
    panel->manager->active_panel = panel;
  }

  unsigned int coords[2] = { x, y };
  LinkedListNode* node = linked_list_find(&panel->actionRegions, coords, coords_over_action_region);

  if (node) {
    ActionRegion* actionRegion = (ActionRegion*)node->data;
    ActionRegionArgs args = {
      panel->owner,
      panel,
      actionRegion,
      x - actionRegion->position[0],
      y - actionRegion->position[1]
    };

    switch (action) {
      case MOUSE_PRESS:
        if (actionRegion->action_press)
          actionRegion->action_press(&args);
        break;
      case MOUSE_RELEASE:
        if (actionRegion->action_release)
          actionRegion->action_release(&args);
        break;
    }
  }
}

void panel_set_position(Panel* panel, int x, int y) {
  panel->position[0] = x;
  panel->position[1] = y;

  GLfloat vertex_data[] =  {
    panel->position[0], panel->position[1], -0.5, 0, 0,
    panel->position[0], panel->position[1] + panel->height, -0.5, 0, 1,
    panel->position[0] + panel->width, panel->position[1], -0.5, 1, 0,
    panel->position[0] + panel->width, panel->position[1] + panel->height, -0.5, 1, 1
  };

  glBindBuffer(GL_ARRAY_BUFFER, panel->vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_data), vertex_data);
}

void panel_translate(Panel* panel, int x, int y) {
  int tx = panel->position[0] + x;
  int ty = panel->position[1] + y;
  panel_set_position(panel, tx, ty);
}

void panel_texture(Panel* panel) {
  glBindTexture(GL_TEXTURE_2D, panel->tex);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  unsigned char* pixels = cairo_image_surface_get_data(panel->surface);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, panel->width, panel->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void panel_draw(Panel* panel) {
  renderer_2D_use(panel->manager->renderer);
  glBindBuffer(GL_ARRAY_BUFFER, panel->vbo);

  glEnableVertexAttribArray(panel->manager->renderer->shaderProgram2D.attrib_position);
  glVertexAttribPointer(panel->manager->renderer->shaderProgram2D.attrib_position, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
  glEnableVertexAttribArray(panel->manager->renderer->shaderProgram2D.attrib_texcoord);
  glVertexAttribPointer(panel->manager->renderer->shaderProgram2D.attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));

  panel->drawCallback(panel->owner);
  panel_texture(panel);
  
  glBindTexture(GL_TEXTURE_2D, panel->tex);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

/* PanelManager */

PanelManager* panel_manager_init(PanelManager* pm, Renderer* renderer) {
  PanelManager* panelManager = pm ? pm : NEW(PanelManager, 1);

  panelManager->renderer = renderer;

  linked_list_init(&panelManager->panels);

  panelManager->active_panel = NULL;
  panelManager->dragging = 0;

  return panelManager;
}

void panel_manager_destroy(PanelManager* panelManager) {
  linked_list_destroy(&panelManager->panels, NULL);
}

void panel_manager_add_panel(PanelManager* panelManager, Panel* panel) {
  linked_list_insert(&panelManager->panels, panel);
  panel->manager = panelManager;
}

Panel* panel_manager_find_panel(PanelManager* panelManager, unsigned int x, unsigned int y) {
  unsigned int coords[2] = { x, y };
  LinkedListNode* node = linked_list_find(&panelManager->panels, coords, coords_over_panel);

  return node ? node->data : NULL;
}

void panel_manager_draw(PanelManager* panelManager) {
  linked_list_foreach(&panelManager->panels, draw_panel, NULL);
}
