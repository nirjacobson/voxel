#ifndef RENDERER_INTERNAL_H
#define RENDERER_INTERNAL_H

#include "../renderer.h"

#include "../ground.h"
#include "../chunk.h"
#include "../mesh.h"
#include "../panel.h"

typedef struct {
    _Alignas(16) float camera[16];
    _Alignas(16) float projection[16];
} UniformBufferObject3D;

typedef struct {
    _Alignas(16) float model[16];
    _Alignas(16) float worldPosition[3];
    _Alignas(16) float color[3];
    _Alignas(16) float sunPosition[3];
    _Alignas(16) float ambient;
} PushConstants3D;

typedef struct {
    _Alignas(16) float projection[16];
} PushConstants2D;

void renderer_3D_create_descriptor_set_layout(Renderer* renderer, VkDescriptorSetLayout* dsLayout);
void renderer_3D_get_binding_description(VkVertexInputBindingDescription* description);
void renderer_3D_get_attribute_descriptions(VkVertexInputAttributeDescription** descriptions, int* count);
void renderer_3D_create_pipeline(Renderer* renderer);

void renderer_2D_create_descriptor_set_layout(Renderer* renderer, VkDescriptorSetLayout* dsLayout);
void renderer_2D_get_binding_description(VkVertexInputBindingDescription* description);
void renderer_2D_get_attribute_descriptions(VkVertexInputAttributeDescription** descriptions, int* count);
void renderer_2D_create_pipeline(Renderer* renderer);

void record_mesh(void* ptr, void* rendererPtr);
void record_world_chunk(void* worldChunkPtr, void* rendererPtr);
void record_panel(void* panelPtr, void* rendererPtr);

void renderer_setup_depth_resources(Renderer* renderer);
void renderer_setup_framebuffers(Renderer* renderer);

void renderer_3D_create_uniform_buffers(Renderer* renderer);
void renderer_create_descriptor_pool(Renderer* renderer);
void renderer_3D_create_descriptor_sets(Renderer* renderer);
void renderer_2D_create_descriptor_sets(Renderer* renderer);

void renderer_create_command_buffers(Renderer* renderer);

void renderer_create_sync_objects(Renderer* renderer);
void renderer_recreate_swap_chain(Renderer* renderer);
void renderer_cleanup_swap_chain(Renderer* renderer);

void renderer_3D_update_uniforms(Renderer* renderer, Camera* camera);

void renderer_record_command_buffer(Renderer* renderer, VkCommandBuffer commandBuffer, uint32_t imageIndex, World* world, Camera* camera, Picker* picker, LinkedList* panels);

void renderer_3D_record_mesh(Renderer* renderer, Mesh* mesh, char mode);
void renderer_3D_record_chunk(Renderer* renderer, Chunk* chunk, float* position);
void renderer_3D_record_picker(Renderer* renderer, Picker* picker);
void renderer_3D_record_ground(Renderer* renderer, Ground* ground, Camera* camera);
void renderer_3D_record(Renderer* renderer, World* world, Camera* camera, Picker* picker);

void renderer_2D_record_panel(Renderer* renderer, Panel* panel);
void renderer_2D_record(Renderer* renderer, LinkedList* panels);

#endif // RENDERER_INTERNAL_H
