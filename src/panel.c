#include "panel.h"
#include "internal/panel.h"

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

/* Panel */

Panel* panel_init(Panel* d, Renderer* renderer, void* owner, void (*drawCallback)(void*), PanelManager* manager, unsigned int width, unsigned int height) {
    Panel* panel = d ? d : NEW(Panel, 1);

    panel->renderer = renderer;
    panel->vulkan = renderer->vulkan;

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

    if (!panel->vulkan) {
        glGenBuffers(1, &panel->renderState.opengl.vbo);
        glGenTextures(1, &panel->renderState.opengl.tex);

        float vertex_data[] =  {
            panel->position[0], panel->position[1], -0.5, 0, 0,
            panel->position[0], panel->position[1] + panel->height, -0.5, 0, 1,
            panel->position[0] + panel->width, panel->position[1], -0.5, 1, 0,
            panel->position[0] + panel->width, panel->position[1] + panel->height, -0.5, 1, 1
        };
        glBindBuffer(GL_ARRAY_BUFFER, panel->renderState.opengl.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
    } else {
        panel_create_vulkan_resources(panel);
    }

    panel_manager_add_panel(panel->manager, panel);

    return panel;
}

void panel_destroy(Panel* panel) {
    if (!panel->vulkan) {
        glDeleteTextures(1, &panel->renderState.opengl.tex);
        glDeleteBuffers(1, &panel->renderState.opengl.vbo);
    } else {
        free(panel->renderState.vulkan.descriptorSets);

        vkDestroyImageView(panel->vulkan->device, panel->renderState.vulkan.texImageView, NULL);
        vkDestroyImage(panel->vulkan->device, panel->renderState.vulkan.texImage, NULL);
        vkFreeMemory(panel->vulkan->device, panel->renderState.vulkan.texImageDeviceMemory, NULL);

        vkDestroyBuffer(panel->vulkan->device, panel->renderState.vulkan.vbo, NULL);
        vkFreeMemory(panel->vulkan->device, panel->renderState.vulkan.vboDeviceMemory, NULL);
    }

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

    float z = panel->vulkan ? 0 : -0.5;

    float vertex_data[] =  {
        panel->position[0], panel->position[1], z, 0, 0,
        panel->position[0], panel->position[1] + panel->height, z, 0, 1,
        panel->position[0] + panel->width, panel->position[1], z, 1, 0,
        panel->position[0] + panel->width, panel->position[1] + panel->height, z, 1, 1
    };


    if (!panel->vulkan) {
        glBindBuffer(GL_ARRAY_BUFFER, panel->renderState.opengl.vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_data), vertex_data);
    } else {
        // Vertex data
        VkDeviceSize bufferSize = sizeof(vertex_data);
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        vulkan_create_buffer(panel->vulkan->physicalDevice, panel->vulkan->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

        void* data;
        vkMapMemory(panel->vulkan->device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertex_data, (size_t)bufferSize);
        vkUnmapMemory(panel->vulkan->device, stagingBufferMemory);

        vulkan_copy_buffer(panel->vulkan->device, panel->vulkan->commandQueue, panel->vulkan->commandPool, stagingBuffer, panel->renderState.vulkan.vbo, bufferSize);

        vkDestroyBuffer(panel->vulkan->device, stagingBuffer, NULL);
        vkFreeMemory(panel->vulkan->device, stagingBufferMemory, NULL);
    }
}

void panel_translate(Panel* panel, int x, int y) {
    int tx = panel->position[0] + x;
    int ty = panel->position[1] + y;
    panel_set_position(panel, tx, ty);
}

void panel_texture(Panel* panel) {
    if (!panel->vulkan) {
        glBindTexture(GL_TEXTURE_2D, panel->renderState.opengl.tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        unsigned char* pixels = cairo_image_surface_get_data(panel->surface);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, panel->width, panel->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    } else {
        unsigned char* pixels = cairo_image_surface_get_data(panel->surface);
        VkDeviceSize imageSize = panel->width * panel->height * 4;
    
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        vulkan_create_buffer(panel->vulkan->physicalDevice, panel->vulkan->device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
    
        void* data;
        vkMapMemory(panel->vulkan->device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, (size_t)(imageSize));
        vkUnmapMemory(panel->vulkan->device, stagingBufferMemory);
    
        vulkan_transition_image_layout(panel->vulkan->device, panel->vulkan->commandQueue, panel->vulkan->commandPool, panel->renderState.vulkan.texImage, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        vulkan_copy_buffer_to_image(panel->vulkan->device, panel->vulkan->commandQueue, panel->vulkan->commandPool, stagingBuffer, panel->renderState.vulkan.texImage, panel->width, panel->height);
        vulkan_transition_image_layout(panel->vulkan->device, panel->vulkan->commandQueue, panel->vulkan->commandPool, panel->renderState.vulkan.texImage, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
        vkDestroyBuffer(panel->vulkan->device, stagingBuffer, NULL);
        vkFreeMemory(panel->vulkan->device, stagingBufferMemory, NULL);
    }
}

void panel_create_vulkan_resources(Panel* panel) {
    float vertex_data[] =  {
        panel->position[0], panel->position[1], 0, 0, 0,
        panel->position[0], panel->position[1] + panel->height, 0, 0, 1,
        panel->position[0] + panel->width, panel->position[1], 0, 1, 0,
        panel->position[0] + panel->width, panel->position[1] + panel->height, 0, 1, 1
    };

    // Vertex data
    VkDeviceSize bufferSize = sizeof(vertex_data);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    vulkan_create_buffer(panel->vulkan->physicalDevice, panel->vulkan->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(panel->vulkan->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertex_data, (size_t)bufferSize);
    vkUnmapMemory(panel->vulkan->device, stagingBufferMemory);

    vulkan_create_buffer(panel->vulkan->physicalDevice, panel->vulkan->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &panel->renderState.vulkan.vbo, &panel->renderState.vulkan.vboDeviceMemory);
    vulkan_copy_buffer(panel->vulkan->device, panel->vulkan->commandQueue, panel->vulkan->commandPool, stagingBuffer, panel->renderState.vulkan.vbo, bufferSize);

    vkDestroyBuffer(panel->vulkan->device, stagingBuffer, NULL);
    vkFreeMemory(panel->vulkan->device, stagingBufferMemory, NULL);

    // Texture
    bufferSize = panel->width * panel->height * 4;
    vulkan_create_image(panel->vulkan->physicalDevice, panel->vulkan->device, panel->width, panel->height, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &panel->renderState.vulkan.texImage, &panel->renderState.vulkan.texImageDeviceMemory);
    panel->renderState.vulkan.texImageView = vulkan_create_image_view(panel->vulkan->device, panel->renderState.vulkan.texImage, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    // Descriptor sets
    panel->renderState.vulkan.descriptorSets = NEW(VkDescriptorSet, MAX_FRAMES_IN_FLIGHT);
    renderer_create_descriptor_sets(panel->renderer, panel->renderState.vulkan.texImageView, &panel->renderState.vulkan.descriptorSets);
}

/* PanelManager */

PanelManager* panel_manager_init(PanelManager* pm) {
    PanelManager* panelManager = pm ? pm : NEW(PanelManager, 1);

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

