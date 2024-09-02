#ifndef VULKAN_UTIL_H
#define VULKAN_UTIL_H

#include "global.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "matrix.h"

typedef struct {
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout layout;
    VkPipeline pipeline;
    int descriptorSetCount;
} Pipeline;

typedef struct {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    VkCommandPool commandPool;
    VkQueue commandQueue;
} Vulkan;

typedef struct {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
} QueueFamilyIndices;

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    VkPresentModeKHR* presentModes;
    int numFormats;
    int numPresentModes;
} SwapChainSupportDetails;

typedef struct {
    VkSwapchainKHR swapChain;
    VkImage* images;
    VkImageView* imageViews;
    VkFramebuffer* frameBuffers;
    int imageCount;
    VkFormat imageFormat;
    VkExtent2D extent;
} SwapChain;

bool vulkan_check_validation_layer_support();
bool vulkan_queue_family_indices_is_complete(QueueFamilyIndices* indices);
QueueFamilyIndices vulkan_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);
SwapChainSupportDetails vulkan_query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface);
bool vulkan_check_device_extension_support(VkPhysicalDevice device);
bool vulkan_is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface);
int vulkan_validation_layers_count();
const char** vulkan_validation_layers();
bool vulkan_create_instance(const char* appName, VkInstance* instance);
void vulkan_pick_physical_device(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* physicalDevice);
void vulkan_create_logical_device(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice* device, VkQueue* graphicsQueue, VkQueue* presentQueue);
VkSurfaceFormatKHR vulkan_choose_swap_surface_format(SwapChainSupportDetails* supportDetails);
VkPresentModeKHR vulkan_choose_swap_present_mode(SwapChainSupportDetails* supportDetails);
VkExtent2D vulkan_choose_swap_extent(SwapChainSupportDetails* supportDetails, GLFWwindow* glfwWindow);
VkImageView vulkan_create_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
void vulkan_create_swap_chain(VkPhysicalDevice physicalDevice, VkDevice device, GLFWwindow* glfwWindow, VkSurfaceKHR surface, SwapChain* swapChain);
VkFormat vulkan_find_supported_format(VkPhysicalDevice physicalDevice, VkFormat* candidates, int count, VkImageTiling tiling, VkFormatFeatureFlags features);
VkFormat vulkan_find_depth_format(VkPhysicalDevice physicalDevice);
void vulkan_create_render_pass(VkPhysicalDevice physicalDevice, VkDevice device, SwapChain* swapChain, VkRenderPass* renderPass);
VkShaderModule vulkan_create_shader_module(VkDevice device, const unsigned char* src, const int len);
void vulkan_create_pipeline(VkDevice device, VkPipelineShaderStageCreateInfo* vertInfo, VkPipelineShaderStageCreateInfo* fragInfo, VkVertexInputBindingDescription* vertexBindingDescription, VkVertexInputAttributeDescription* vertexAttributeDescriptions, int vertexAttributeDescriptionCount, VkPushConstantRange* pushConstants, VkRenderPass renderPass, VkPrimitiveTopology topology, Pipeline* pipeline);
void vulkan_create_command_pool(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkCommandPool* commandPool);
uint32_t vulkan_find_memory_type(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void vulkan_create_image(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory);
void vulkan_create_sampler(VkPhysicalDevice physicalDevice, VkDevice device, VkSampler* sampler);
void vulkan_create_buffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
void vulkan_create_descriptor_set_pool(VkDevice device, VkDescriptorPoolSize* poolSizes, int numPoolSizes, VkDescriptorPool* descriptorPool, uint32_t maxSets);
void vulkan_create_descriptor_sets(VkDevice device, VkDescriptorPool descriptorPool, int count, VkDescriptorSetLayout layout, VkDescriptorSet** descriptorSets);
void vulkan_create_command_buffers(VkDevice device, VkCommandPool commandPool, int count, VkCommandBuffer* commandBuffers);
void vulkan_copy_buffer(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void vulkan_transition_image_layout(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
void vulkan_copy_buffer_to_image(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

void vulkan_clip_correction_matrix(float* data);

#endif // VULKAN_UTIL_H