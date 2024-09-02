#ifndef MESH_H
#define MESH_H

#include <stdlib.h>
#include <stdint.h>

#include <GLES3/gl3.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "global.h"
#include "linked_list.h"
#include "vulkan_util.h"

#define NORTH           0
#define SOUTH           1
#define WEST            2
#define EAST            3
#define TOP             4
#define BOTTOM          5

#define MESH_FILL       1
#define MESH_LINE       2

typedef struct {
    float position[3];
    float normal_v[3];
} Vertex;

typedef struct {
    Vertex vertices[4];
    char orientation;
} Quad;

typedef struct {
    Vulkan* vulkan;
    uint16_t color;
    LinkedList quads;

    union {
        struct {
            GLuint vbo;
            GLuint ebo;
        } opengl;
        struct {
            VkBuffer vbo;
            VkDeviceMemory vboDeviceMemory;
            VkBuffer ebo;
            VkDeviceMemory eboDeviceMemory;
            bool haveBuffers;
        } vulkan;
    } renderState;
} Mesh;

Mesh* mesh_init(Mesh* m, Vulkan* vulkan);
void mesh_destroy(Mesh* mesh);

void mesh_add_quad(Mesh* mesh, Quad* quad);
void mesh_calc_normals(Mesh* mesh);

void mesh_buffer(Mesh* mesh, char mode);

#endif // MESH_H
