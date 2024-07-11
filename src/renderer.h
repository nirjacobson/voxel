#ifndef RENDERER_H
#define RENDERER_H

#include <math.h>
#include <gio/gio.h>

#include "matrix.h"

#include "camera.h"
#include "world.h"
#include "picker.h"
#include "window.h"

#include "vulkan_util.h"

typedef struct {
    Pipeline pipeline;
    Pipeline pipelineLine;
    VkBuffer* mcpBuffers;
    VkDeviceMemory* mcpBuffersMemory;
    void** mcpBuffersMapped;
    VkDescriptorSet* descriptorSets;
} Pipeline3D;

typedef struct {
    Pipeline pipeline;
    VkSampler sampler;
} Pipeline2D;

typedef struct {
    Window* window;
    Vulkan* vulkan;

    Pipeline3D pipeline3D;
    Pipeline2D pipeline2D;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    SwapChain swapChain;
    VkRenderPass renderPass;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    VkDescriptorPool descriptorPool;
    VkCommandBuffer* commandBuffers;
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* inFlightFences;

    uint32_t currentFrame;
    bool framebufferResized;
} Renderer;

Renderer* renderer_init(Renderer* r, Window* window, Vulkan* vulkan);
void renderer_destroy(Renderer* renderer);

void renderer_resize(Renderer* renderer);

void renderer_render(Renderer* renderer, World* world, Camera* camera, Picker* picker, LinkedList* panels);

void renderer_create_descriptor_sets(Renderer* renderer, VkImageView imageView, VkDescriptorSet** descriptorSets);

#endif // RENDERER_H
