#include "cursor.h"


Cursor* cursor_init(Cursor* c) {
    Cursor* cursor = c ? c : NEW(Cursor, 1);

    glGenBuffers(1, &cursor->vbo);

    float xf = 10;
    float yf = 10;
    float data[] = {
        xf, yf, 0.0f, 0.0f, 0.0f,
        xf, yf + CURSOR_HEIGHT, 0.0f, 0.0f, 1.0f,
        xf + CURSOR_WIDTH, yf, 0.0f, 1.0f, 0.0f,
        xf + CURSOR_WIDTH, yf + CURSOR_HEIGHT, 0.0f, 1.0f, 1.0f,
    };
    glBindBuffer(GL_ARRAY_BUFFER, cursor->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    glGenTextures(1, &cursor->tex);

    
    cairo_surface_t* surface = cairo_image_surface_create_from_png("img/cursor.png");
    unsigned char* pixels = cairo_image_surface_get_data(surface);

    glBindTexture(GL_TEXTURE_2D, cursor->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CURSOR_WIDTH, CURSOR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);

    cairo_surface_destroy(surface);

    return cursor;
}

void cursor_destroy(Cursor* cursor) {
    glDeleteTextures(1, &cursor->tex);
    glDeleteBuffers(1, &cursor->vbo);
}

void cursor_set_position(Cursor* cursor, int x, int y) {
    cursor->position[0] = x;
    cursor->position[1] = y;
    float xf = x;
    float yf = y;
    float data[] = {
        xf, yf, -1.0f, 0.0f, 0.0f,
        xf, yf + CURSOR_HEIGHT, -1.0f, 0.0f, 1.0f,
        xf + CURSOR_WIDTH, yf, -1.0f, 1.0f, 0.0f,
        xf + CURSOR_WIDTH, yf + CURSOR_HEIGHT, -1.0f, 1.0f, 1.0f,
    };

    glBindBuffer(GL_ARRAY_BUFFER, cursor->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
}

void cursor_draw(Cursor* cursor, Renderer* renderer) {
    renderer_2D_use(renderer);
    glBindBuffer(GL_ARRAY_BUFFER, cursor->vbo);
    glBindTexture(GL_TEXTURE_2D, cursor->tex);

    glEnableVertexAttribArray(renderer->shaderProgram2D.attrib_position);
    glVertexAttribPointer(renderer->shaderProgram2D.attrib_position, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(renderer->shaderProgram2D.attrib_texcoord);
    glVertexAttribPointer(renderer->shaderProgram2D.attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}