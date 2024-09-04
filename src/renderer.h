#ifndef RENDERER_H
#define RENDERER_H

#include <math.h>
#include <gio/gio.h>
#include <GL/glew.h>

#include "shader.h"
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
    union {
        struct {
            ShaderProgram3D shaderProgram3D;
            ShaderProgram2D shaderProgram2D;
        } opengl;
        struct {
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
        } vulkan;
    } renderState;
    
} Renderer;

Renderer* renderer_init(Renderer* r, Window* window, Vulkan* vulkan);
void renderer_destroy(Renderer* renderer);

/* OpenGL */
void renderer_clear(Renderer* renderer);
void renderer_resize(Renderer* renderer, int width, int height, Camera* camera);
void renderer_apply_camera(Renderer* renderer, Camera* camera);
void renderer_render_world(Renderer* renderer, World* world, Camera* camera);
void renderer_render_picker(Renderer* renderer, Picker* picker);
void renderer_render_panels(Renderer* renderer, LinkedList* panels);

/* Vulkan */
void renderer_vulkan_resize(Renderer* renderer);

void renderer_vulkan_render(Renderer* renderer, World* world, Camera* camera, Picker* picker, LinkedList* panels);

void renderer_create_descriptor_sets(Renderer* renderer, VkImageView imageView, VkDescriptorSet** descriptorSets);

#endif // RENDERER_H
