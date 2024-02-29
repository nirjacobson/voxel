#include "mesh.h"
#include "internal/mesh.h"

/* Helpers */

void mesh_ortn_to_normal(char orientation, float* nvec) {
    switch (orientation) {
        case NORTH:
            nvec[0] =  0;
            nvec[1] =  0;
            nvec[2] = -1;
            break;
        case SOUTH:
            nvec[0] =  0;
            nvec[1] =  0;
            nvec[2] =  1;
            break;
        case WEST:
            nvec[0] = -1;
            nvec[1] =  0;
            nvec[2] =  0;
            break;
        case EAST:
            nvec[0] =  1;
            nvec[1] =  0;
            nvec[2] =  0;
            break;
        case TOP:
            nvec[0] =  0;
            nvec[1] =  1;
            nvec[2] =  0;
            break;
        case BOTTOM:
            nvec[0] =  0;
            nvec[1] = -1;
            nvec[2] =  0;
        default:
            break;
    }
}

void mesh_add_quad(Mesh* mesh, Quad* quad) {
    linked_list_insert(&mesh->quads, quad);
}

void quad_set_normals(void* ptr, void* unused) {
    Quad* quad = (Quad*)ptr;

    float normal[3];
    mesh_ortn_to_normal(quad->orientation, normal);
    for (int v = 0; v < 4; v++) {
        for (int i = 0; i < 3; i++) {
            quad->vertices[v].normal_v[i] = normal[i];
        }
    }
}

/* Mesh */

Mesh* mesh_init(Mesh* m, Vulkan* vulkan) {
    Mesh* mesh = m ? m : NEW(Mesh, 1);

    mesh->vulkan = vulkan;

    linked_list_init(&mesh->quads);

    mesh->haveBuffers = false;

    return mesh;
}

void mesh_destroy(Mesh* mesh) {
    vkQueueWaitIdle(mesh->vulkan->commandQueue);

    vkDestroyBuffer(mesh->vulkan->device, mesh->ebo, NULL);
    vkFreeMemory(mesh->vulkan->device, mesh->eboDeviceMemory, NULL);
    vkDestroyBuffer(mesh->vulkan->device, mesh->vbo, NULL);
    vkFreeMemory(mesh->vulkan->device, mesh->vboDeviceMemory, NULL);

    linked_list_destroy(&mesh->quads, free);
}

void mesh_calc_normals(Mesh* mesh) {
    linked_list_foreach(&mesh->quads, quad_set_normals, NULL);
}

void mesh_buffer(Mesh* mesh, char mode) {
    int num_elements_f  = mesh->quads.size * 4;
    int num_vertices_f  = num_elements_f * 6;
    float* vertex_data  = NEW(float, num_vertices_f);
    uint16_t*  elements = NEW(uint16_t, num_elements_f);

    LinkedListNode* node = mesh->quads.head;
    for (int q=0; node; q++, node = node->next) {
        Quad* quad = (Quad*)node->data;
        for (int v=0; v<4; v++) {
            for (int p=0; p<3; p++)
                vertex_data[q*24+v*6+0+p] = quad->vertices[v].position[p];
            for (int n=0; n<3; n++)
                vertex_data[q*24+v*6+3+n] = quad->vertices[v].normal_v[n];

            int order[] = {
                0,
                1,
                mode == MESH_FILL ? 2 : 3,
                mode == MESH_FILL ? 3 : 2,
            };

            elements[q*4+v] = q*4 + order[v];
        }
    }

    // Vertices
    VkBuffer oldBuffer = mesh->vbo;
    VkDeviceMemory oldMemory = mesh->vboDeviceMemory;

    VkDeviceSize bufferSize = num_vertices_f*sizeof(float);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    vulkan_create_buffer(mesh->vulkan->physicalDevice, mesh->vulkan->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(mesh->vulkan->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertex_data, bufferSize);
    vkUnmapMemory(mesh->vulkan->device, stagingBufferMemory);

    vulkan_create_buffer(mesh->vulkan->physicalDevice, mesh->vulkan->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mesh->vbo, &mesh->vboDeviceMemory);
    vulkan_copy_buffer(mesh->vulkan->device, mesh->vulkan->commandQueue, mesh->vulkan->commandPool, stagingBuffer, mesh->vbo, bufferSize);

    vkDestroyBuffer(mesh->vulkan->device, stagingBuffer, NULL);
    vkFreeMemory(mesh->vulkan->device, stagingBufferMemory, NULL);

    if (mesh->haveBuffers) {
        vkQueueWaitIdle(mesh->vulkan->commandQueue);
        vkDestroyBuffer(mesh->vulkan->device, oldBuffer, NULL);
        vkFreeMemory(mesh->vulkan->device, oldMemory, NULL);
    }

    // Indices
    oldBuffer = mesh->ebo;
    oldMemory = mesh->eboDeviceMemory;

    bufferSize = num_elements_f*sizeof(uint16_t);
    vulkan_create_buffer(mesh->vulkan->physicalDevice, mesh->vulkan->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(mesh->vulkan->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, elements, bufferSize);
    vkUnmapMemory(mesh->vulkan->device, stagingBufferMemory);

    vulkan_create_buffer(mesh->vulkan->physicalDevice, mesh->vulkan->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mesh->ebo, &mesh->eboDeviceMemory);
    vulkan_copy_buffer(mesh->vulkan->device, mesh->vulkan->commandQueue, mesh->vulkan->commandPool, stagingBuffer, mesh->ebo, bufferSize);

    vkDestroyBuffer(mesh->vulkan->device, stagingBuffer, NULL);
    vkFreeMemory(mesh->vulkan->device, stagingBufferMemory, NULL);

    if (mesh->haveBuffers) {
        vkQueueWaitIdle(mesh->vulkan->commandQueue);
        vkDestroyBuffer(mesh->vulkan->device, oldBuffer, NULL);
        vkFreeMemory(mesh->vulkan->device, oldMemory, NULL);
    }

    mesh->haveBuffers = true;

    free(elements);
    free(vertex_data);
}

