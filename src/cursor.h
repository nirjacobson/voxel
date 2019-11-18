#ifndef CURSOR_H
#define CURSOR_H

#define CURSOR_WIDTH    16
#define CURSOR_HEIGHT   16
#define CURSOR_BPP       4
#define CURSOR_SIZE     (CURSOR_WIDTH * CURSOR_HEIGHT * CURSOR_BPP)

#include <string.h>
#include <assert.h>
#include <cairo/cairo.h>

#include "global.h"
#include "renderer.h"

typedef struct {
    int position[2];
    GLuint vbo;
    GLuint tex;
} Cursor;

Cursor* cursor_init(Cursor* c);
void cursor_destroy(Cursor* cursor);

void cursor_set_position(Cursor* cursor, int x, int y);
void cursor_draw(Cursor* cursor, Renderer* renderer);

#endif // CURSOR_H
