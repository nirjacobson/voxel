#ifndef RENDERER_INTERNAL_H
#define RENDERER_INTERNAL_H

#include "../renderer.h"
#include "../panel.h"

void render_mesh(void* ptr, void* rendererPtr);
void render_world_chunk(void* worldChunkPtr, void* rendererPtr);
void render_panel(void* panelPtr, void* rendererPtr);

void renderer_render_panel(Renderer* renderer, Panel* panel);

#endif // RENDERER_INTERNAL_H
