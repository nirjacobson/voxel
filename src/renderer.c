#include "renderer.h"
#include "internal/renderer.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

void record_mesh(void* ptr, void* rendererPtr) {
    Mesh* mesh = (Mesh*)ptr;
    Renderer* renderer = (Renderer*)rendererPtr;

    float color[3];
    block_color_rgb(mesh->color, color);
    color[0] /= 255.0f;
    color[1] /= 255.0f;
    color[2] /= 255.0f;

    // Color
    vkCmdPushConstants(renderer->commandBuffers[renderer->currentFrame], renderer->pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 80, sizeof(color), color);

    renderer_3D_record_mesh(renderer, mesh, MESH_FILL);
}

void record_world_chunk(void* worldChunkPtr, void* rendererPtr) {
    WorldChunk* worldChunk = (WorldChunk*)worldChunkPtr;
    Renderer* renderer = (Renderer*)rendererPtr;
    float position[3] = {
        worldChunk->id.x * WORLD_CHUNK_LENGTH,
        worldChunk->id.y * WORLD_CHUNK_LENGTH,
        worldChunk->id.z * WORLD_CHUNK_LENGTH
    };

    if (worldChunk->chunk->meshes.size)
        renderer_3D_record_chunk(renderer, worldChunk->chunk, position);
}

void record_panel(void* panelPtr, void* rendererPtr) {
    Panel* panel = (Panel*)panelPtr;
    Renderer* renderer = (Renderer*)rendererPtr;
    renderer_2D_record_panel(renderer, panel);
}

void renderer_2D_record_panel(Renderer* renderer, Panel* panel) {
    VkBuffer vertexBuffers[] = { panel->vbo };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindDescriptorSets(renderer->commandBuffers[renderer->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline2D.pipeline.layout, 0, 1, &panel->descriptorSets[renderer->currentFrame], 0, NULL);
    vkCmdBindVertexBuffers(renderer->commandBuffers[renderer->currentFrame], 0, 1, vertexBuffers, offsets);
    vkCmdDraw(renderer->commandBuffers[renderer->currentFrame], 4, 1, 0, 0);
}

void renderer_3D_create_descriptor_set_layout(Renderer* renderer, VkDescriptorSetLayout* dsLayout) {
    VkDescriptorSetLayoutBinding mcpLayoutBinding = { 0 };
    mcpLayoutBinding.binding = 0;
    mcpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    mcpLayoutBinding.descriptorCount = 1;
    mcpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    mcpLayoutBinding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo layoutInfo = { 0 };
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &mcpLayoutBinding;

    if (vkCreateDescriptorSetLayout(renderer->vulkan->device, &layoutInfo, NULL, dsLayout) != VK_SUCCESS) {
        printf("failed to create descriptor set layout.\n");
        assert(false);
    }
}

void renderer_setup_depth_resources(Renderer* renderer) {
    VkFormat formats[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};

    VkFormat depthFormat = vulkan_find_supported_format(
                               renderer->vulkan->physicalDevice,
                               formats,
                               sizeof(formats) / sizeof(formats[0]),
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
                           );

    vulkan_create_image(renderer->vulkan->physicalDevice,
                        renderer->vulkan->device,
                        renderer->swapChain.extent.width,
                        renderer->swapChain.extent.height,
                        depthFormat,
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        &renderer->depthImage,
                        &renderer->depthImageMemory);
    renderer->depthImageView = vulkan_create_image_view(renderer->vulkan->device, renderer->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

}

void renderer_setup_framebuffers(Renderer* renderer) {
    for (int i = 0; i < renderer->swapChain.imageCount; i++) {
        VkImageView attachments[] = { renderer->swapChain.imageViews[i], renderer->depthImageView };

        VkFramebufferCreateInfo frameBufferInfo = { 0 };
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = renderer->renderPass;
        frameBufferInfo.attachmentCount = (uint32_t)(sizeof(attachments)/sizeof(attachments[0]));
        frameBufferInfo.pAttachments = attachments;
        frameBufferInfo.width = renderer->swapChain.extent.width;
        frameBufferInfo.height = renderer->swapChain.extent.height;
        frameBufferInfo.layers = 1;

        if (vkCreateFramebuffer(renderer->vulkan->device, &frameBufferInfo, NULL, &renderer->swapChain.frameBuffers[i]) != VK_SUCCESS) {
            printf("failed to create framebuffer.\n");
            assert(false);
        }
    }
}

Renderer* renderer_init(Renderer* r, Window* window, Vulkan* vulkan) {
    Renderer* renderer = r ? r : NEW(Renderer, 1);

    renderer->window = window;
    renderer->vulkan = vulkan;

    vulkan_create_swap_chain(vulkan->physicalDevice, vulkan->device, window->glfwWindow, window->surface, &renderer->swapChain);

    vulkan_create_render_pass(vulkan->physicalDevice, vulkan->device, &renderer->swapChain, &renderer->renderPass);

    renderer_3D_create_pipeline(renderer);
    renderer_2D_create_pipeline(renderer);

    renderer_setup_depth_resources(renderer);

    renderer_setup_framebuffers(renderer);

    vulkan_create_sampler(vulkan->physicalDevice, vulkan->device, &renderer->pipeline2D.sampler);

    renderer_3D_create_uniform_buffers(renderer);

    renderer_create_descriptor_pool(renderer);

    renderer_3D_create_descriptor_sets(renderer);


    renderer_create_command_buffers(renderer);

    renderer_create_sync_objects(renderer);

    renderer->currentFrame = 0;
    renderer->framebufferResized = false;

    return renderer;
}

void renderer_3D_get_binding_description(VkVertexInputBindingDescription* description) {
    VkVertexInputBindingDescription bindingDescription = { 0 };
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    *description = bindingDescription;
}

void renderer_3D_get_attribute_descriptions(VkVertexInputAttributeDescription** descriptions, int* count) {
    VkVertexInputAttributeDescription attributeDescriptions[2];

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = 0;

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = 3 * sizeof(float);

    *descriptions = NEW(VkVertexInputAttributeDescription, 2);
    *count = 2;

    memcpy(*descriptions, attributeDescriptions, sizeof(attributeDescriptions));
}

void renderer_3D_create_pipeline(Renderer* renderer) {
    renderer_3D_create_descriptor_set_layout(renderer, &renderer->pipeline3D.pipeline.descriptorSetLayout);
    renderer_3D_create_descriptor_set_layout(renderer, &renderer->pipeline3D.pipelineLine.descriptorSetLayout);

    GBytes* _3D_vert_spv = g_resources_lookup_data("/build/shaders/3D.vert.spv", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
    GBytes* _3D_frag_spv = g_resources_lookup_data("/build/shaders/3D.frag.spv", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);

    const guint8* __3D_vert_spv = g_bytes_get_data(_3D_vert_spv, NULL);
    gsize __3D_vert_spv_len = g_bytes_get_size(_3D_vert_spv);
    const guint8* __3D_frag_spv = g_bytes_get_data(_3D_frag_spv, NULL);
    gsize __3D_frag_spv_len = g_bytes_get_size(_3D_frag_spv);

    VkShaderModule vertShaderModule = vulkan_create_shader_module(renderer->vulkan->device, __3D_vert_spv, __3D_vert_spv_len);
    VkShaderModule fragShaderModule = vulkan_create_shader_module(renderer->vulkan->device, __3D_frag_spv, __3D_frag_spv_len);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = { 0 };
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = { 0 };
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkVertexInputBindingDescription bindingDescription;
    renderer_3D_get_binding_description(&bindingDescription);

    VkVertexInputAttributeDescription* attributeDescriptions;
    int attributeDescriptionsCount;
    renderer_3D_get_attribute_descriptions(&attributeDescriptions, &attributeDescriptionsCount);

    VkPushConstantRange pushConstants = { 0 };
    pushConstants.offset = 0;
    pushConstants.size = sizeof(PushConstants3D);
    pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    vulkan_create_pipeline(renderer->vulkan->device,
                           &vertShaderStageInfo,
                           &fragShaderStageInfo,
                           &bindingDescription,
                           attributeDescriptions,
                           attributeDescriptionsCount,
                           &pushConstants,
                           renderer->renderPass,
                           VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                           &renderer->pipeline3D.pipeline);
    vulkan_create_pipeline(renderer->vulkan->device,
                           &vertShaderStageInfo,
                           &fragShaderStageInfo,
                           &bindingDescription,
                           attributeDescriptions,
                           attributeDescriptionsCount,
                           &pushConstants,
                           renderer->renderPass,
                           VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
                           &renderer->pipeline3D.pipelineLine);

    vkDestroyShaderModule(renderer->vulkan->device, fragShaderModule, NULL);
    vkDestroyShaderModule(renderer->vulkan->device, vertShaderModule, NULL);

    free(attributeDescriptions);
}

void renderer_2D_create_descriptor_set_layout(Renderer* renderer, VkDescriptorSetLayout* dsLayout) {
    VkDescriptorSetLayoutBinding samplerLayoutBinding = { 0 };
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo layoutInfo = { 0 };
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerLayoutBinding;

    if (vkCreateDescriptorSetLayout(renderer->vulkan->device, &layoutInfo, NULL, dsLayout) != VK_SUCCESS) {
        printf("failed to create descriptor set layout.\n");
        assert(false);
    }
}

void renderer_2D_get_binding_description(VkVertexInputBindingDescription* description) {
    VkVertexInputBindingDescription bindingDescription = { 0 };
    bindingDescription.binding = 0;
    bindingDescription.stride = 5 * sizeof(float);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    *description = bindingDescription;
}

void renderer_2D_get_attribute_descriptions(VkVertexInputAttributeDescription** descriptions, int* count) {
    VkVertexInputAttributeDescription attributeDescriptions[2];

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = 0;

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = 3 * sizeof(float);

    *descriptions = NEW(VkVertexInputAttributeDescription, 2);
    *count = 2;

    memcpy(*descriptions, attributeDescriptions, sizeof(attributeDescriptions));
}

void renderer_2D_create_pipeline(Renderer* renderer) {
    renderer_2D_create_descriptor_set_layout(renderer, &renderer->pipeline2D.pipeline.descriptorSetLayout);

    GBytes* _2D_vert_spv = g_resources_lookup_data("/build/shaders/2D.vert.spv", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
    GBytes* _2D_frag_spv = g_resources_lookup_data("/build/shaders/2D.frag.spv", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);

    const guint8* __2D_vert_spv = g_bytes_get_data(_2D_vert_spv, NULL);
    gsize __2D_vert_spv_len = g_bytes_get_size(_2D_vert_spv);
    const guint8* __2D_frag_spv = g_bytes_get_data(_2D_frag_spv, NULL);
    gsize __2D_frag_spv_len = g_bytes_get_size(_2D_frag_spv);

    VkShaderModule vertShaderModule = vulkan_create_shader_module(renderer->vulkan->device, __2D_vert_spv, __2D_vert_spv_len);
    VkShaderModule fragShaderModule = vulkan_create_shader_module(renderer->vulkan->device, __2D_frag_spv, __2D_frag_spv_len);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = { 0 };
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = { 0 };
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkVertexInputBindingDescription bindingDescription;
    renderer_2D_get_binding_description(&bindingDescription);

    VkVertexInputAttributeDescription* attributeDescriptions;
    int attributeDescriptionsCount;
    renderer_2D_get_attribute_descriptions(&attributeDescriptions, &attributeDescriptionsCount);

    VkPushConstantRange pushConstants = { 0 };
    pushConstants.offset = 0;
    pushConstants.size = sizeof(PushConstants2D);
    pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    vulkan_create_pipeline(renderer->vulkan->device,
                           &vertShaderStageInfo,
                           &fragShaderStageInfo,
                           &bindingDescription,
                           attributeDescriptions,
                           attributeDescriptionsCount,
                           &pushConstants,
                           renderer->renderPass,
                           VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                           &renderer->pipeline2D.pipeline);

    vkDestroyShaderModule(renderer->vulkan->device, fragShaderModule, NULL);
    vkDestroyShaderModule(renderer->vulkan->device, vertShaderModule, NULL);

    free(attributeDescriptions);
}

void renderer_destroy(Renderer* renderer) {
    vkDestroySampler(renderer->vulkan->device, renderer->pipeline2D.sampler, NULL);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkUnmapMemory(renderer->vulkan->device, renderer->pipeline3D.mcpBuffersMemory[i]);
        vkDestroyBuffer(renderer->vulkan->device, renderer->pipeline3D.mcpBuffers[i], NULL);
        vkFreeMemory(renderer->vulkan->device, renderer->pipeline3D.mcpBuffersMemory[i], NULL);
    }
    free(renderer->pipeline3D.descriptorSets);

    vkFreeCommandBuffers(renderer->vulkan->device, renderer->vulkan->commandPool, MAX_FRAMES_IN_FLIGHT, renderer->commandBuffers);
    free(renderer->commandBuffers);

    vkDestroyDescriptorPool(renderer->vulkan->device, renderer->descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(renderer->vulkan->device, renderer->pipeline3D.pipeline.descriptorSetLayout, NULL);
    vkDestroyDescriptorSetLayout(renderer->vulkan->device, renderer->pipeline3D.pipelineLine.descriptorSetLayout, NULL);
    vkDestroyDescriptorSetLayout(renderer->vulkan->device, renderer->pipeline2D.pipeline.descriptorSetLayout, NULL);

    vkDestroyPipeline(renderer->vulkan->device, renderer->pipeline2D.pipeline.pipeline, NULL);
    vkDestroyPipelineLayout(renderer->vulkan->device, renderer->pipeline2D.pipeline.layout, NULL);
    vkDestroyPipeline(renderer->vulkan->device, renderer->pipeline3D.pipeline.pipeline, NULL);
    vkDestroyPipelineLayout(renderer->vulkan->device, renderer->pipeline3D.pipeline.layout, NULL);
    vkDestroyPipeline(renderer->vulkan->device, renderer->pipeline3D.pipelineLine.pipeline, NULL);
    vkDestroyPipelineLayout(renderer->vulkan->device, renderer->pipeline3D.pipelineLine.layout, NULL);

    vkDestroyRenderPass(renderer->vulkan->device, renderer->renderPass, NULL);

    renderer_cleanup_swap_chain(renderer);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(renderer->vulkan->device, renderer->imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(renderer->vulkan->device, renderer->renderFinishedSemaphores[i], NULL);
        vkDestroyFence(renderer->vulkan->device, renderer->inFlightFences[i], NULL);
    }
}

void renderer_resize(Renderer* renderer) {
    renderer->framebufferResized = true;
}

void renderer_3D_record_chunk(Renderer* renderer, Chunk* chunk, float* position) {
    // World position
    vkCmdPushConstants(renderer->commandBuffers[renderer->currentFrame], renderer->pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 64, sizeof(float[3]), position);

    linked_list_foreach(&chunk->meshes, record_mesh, renderer);
}

void renderer_3D_record_picker(Renderer* renderer, Picker* picker) {
    float mat[16];
    float vec[3];

    if (picker->selection.model) {
        mat4_rotate(mat, NULL, (M_PI/2) * picker->selection.rotation, Y);
        switch (picker->selection.rotation) {
            case 0:
                break;
            case 1:
                vec[0] = -1;
                vec[1] = 0;
                vec[2] = 0;
                mat4_translate(mat, mat, vec);
                break;
            case 2:
                vec[0] = -1;
                vec[1] = 0;
                vec[2] = -1;
                mat4_translate(mat, mat, vec);
                break;
            case 3:
                vec[0] = 0;
                vec[1] = 0;
                vec[2] = -1;
                mat4_translate(mat, mat, vec);
                break;
            default:
                break;
        }

        // Model matrix
        vkCmdPushConstants(renderer->commandBuffers[renderer->currentFrame], renderer->pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(mat), mat);

        renderer_3D_record_chunk(renderer, picker->selection.model, picker->box.position);
    }

    vkCmdBindPipeline(renderer->commandBuffers[renderer->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline3D.pipelineLine.pipeline);
    vkCmdBindDescriptorSets(renderer->commandBuffers[renderer->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline3D.pipelineLine.layout, 0, 1, &renderer->pipeline3D.descriptorSets[renderer->currentFrame], 0, NULL);

    if (picker->selection.present) {
        float color[] = { 0, 1, 1 };

        // World position
        vkCmdPushConstants(renderer->commandBuffers[renderer->currentFrame], renderer->pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 64, sizeof(picker->selection.box.position),  picker->selection.box.position);
        // Color
        vkCmdPushConstants(renderer->commandBuffers[renderer->currentFrame], renderer->pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 80, sizeof(color), color);

        renderer_3D_record_mesh(renderer, &picker->selection.mesh, MESH_LINE);
    }

    mat4_identity(mat);
    // Model matrix
    vkCmdPushConstants(renderer->commandBuffers[renderer->currentFrame], renderer->pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(mat), mat);
    // World position
    vkCmdPushConstants(renderer->commandBuffers[renderer->currentFrame], renderer->pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 64, sizeof(picker->box.position),  picker->box.position);
    // Color
    float color[] = { 1, 1, 0 };
    vkCmdPushConstants(renderer->commandBuffers[renderer->currentFrame], renderer->pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 80, sizeof(color), color);

    renderer_3D_record_mesh(renderer, &picker->mesh, MESH_LINE);
}

void renderer_record_panels(Renderer* renderer, LinkedList* panels) {
    linked_list_foreach(panels, record_panel, renderer);
}

void renderer_3D_create_uniform_buffers(Renderer* renderer) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject3D);

    renderer->pipeline3D.mcpBuffers = NEW(VkBuffer, MAX_FRAMES_IN_FLIGHT);
    renderer->pipeline3D.mcpBuffersMemory = NEW(VkDeviceMemory, MAX_FRAMES_IN_FLIGHT);
    renderer->pipeline3D.mcpBuffersMapped = NEW(void*, MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vulkan_create_buffer(renderer->vulkan->physicalDevice,
                             renderer->vulkan->device, bufferSize,
                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             &renderer->pipeline3D.mcpBuffers[i],
                             &renderer->pipeline3D.mcpBuffersMemory[i]);
        vkMapMemory(renderer->vulkan->device, renderer->pipeline3D.mcpBuffersMemory[i], 0, bufferSize, 0, &renderer->pipeline3D.mcpBuffersMapped[i]);
    }
}

void renderer_create_descriptor_pool(Renderer* renderer) {
    VkDescriptorPoolSize poolSizes[2] = { 0 };
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (uint32_t)(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = (uint32_t)(8);

    vulkan_create_descriptor_set_pool(renderer->vulkan->device, poolSizes, sizeof(poolSizes)/sizeof(poolSizes[0]), &renderer->descriptorPool, 8);
}

void renderer_3D_create_descriptor_sets(Renderer* renderer) {
    vulkan_create_descriptor_sets(renderer->vulkan->device, renderer->descriptorPool, MAX_FRAMES_IN_FLIGHT, renderer->pipeline3D.pipeline.descriptorSetLayout, &renderer->pipeline3D.descriptorSets);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo = { 0 };
        bufferInfo.buffer = renderer->pipeline3D.mcpBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject3D);

        VkWriteDescriptorSet descriptorWrite = { 0 };
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = renderer->pipeline3D.descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = NULL;
        descriptorWrite.pTexelBufferView = NULL;

        vkUpdateDescriptorSets(renderer->vulkan->device, 1, &descriptorWrite, 0, NULL);
    }
}

void renderer_create_command_buffers(Renderer* renderer) {
    renderer->commandBuffers = NEW(VkCommandBuffer, MAX_FRAMES_IN_FLIGHT);
    vulkan_create_command_buffers(renderer->vulkan->device, renderer->vulkan->commandPool, MAX_FRAMES_IN_FLIGHT, renderer->commandBuffers);
}

void renderer_create_sync_objects(Renderer* renderer) {
    renderer->imageAvailableSemaphores = NEW(VkSemaphore, MAX_FRAMES_IN_FLIGHT);
    renderer->renderFinishedSemaphores = NEW(VkSemaphore, MAX_FRAMES_IN_FLIGHT);
    renderer->inFlightFences = NEW(VkFence, MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = { 0 };
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = { 0 };
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(renderer->vulkan->device, &semaphoreInfo, NULL, &renderer->imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(renderer->vulkan->device, &semaphoreInfo, NULL, &renderer->renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(renderer->vulkan->device, &fenceInfo, NULL, &renderer->inFlightFences[i]) != VK_SUCCESS) {
            printf("failed to create sync objects.\n");
            assert(false);
        }
    }
}

void renderer_cleanup_swap_chain(Renderer* renderer) {
    vkDestroyImageView(renderer->vulkan->device, renderer->depthImageView, NULL);
    vkDestroyImage(renderer->vulkan->device, renderer->depthImage, NULL);
    vkFreeMemory(renderer->vulkan->device, renderer->depthImageMemory, NULL);
    for (int i = 0; i < renderer->swapChain.imageCount; i++) {
        vkDestroyFramebuffer(renderer->vulkan->device, renderer->swapChain.frameBuffers[i], NULL);
    }
    for (int i = 0; i < renderer->swapChain.imageCount; i++) {
        vkDestroyImageView(renderer->vulkan->device, renderer->swapChain.imageViews[i], NULL);
    }
    free(renderer->swapChain.frameBuffers);
    vkDestroySwapchainKHR(renderer->vulkan->device, renderer->swapChain.swapChain, NULL);
}

void renderer_recreate_swap_chain(Renderer* renderer) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(renderer->window->glfwWindow, &width, &height);
    while (width == 0 && height == 0) {
        glfwGetFramebufferSize(renderer->window->glfwWindow, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(renderer->vulkan->device);

    renderer_cleanup_swap_chain(renderer);

    vulkan_create_swap_chain(renderer->vulkan->physicalDevice, renderer->vulkan->device, renderer->window->glfwWindow, renderer->window->surface, &renderer->swapChain);
    renderer_setup_depth_resources(renderer);
    renderer_setup_framebuffers(renderer);
}

void renderer_3D_update_uniforms(Renderer* renderer, Camera* camera) {
    UniformBufferObject3D ubo = { 0 };

    mat4_multiply(ubo.camera, camera->mat_view, camera->mat_model);
    mat4_inverse(ubo.camera, ubo.camera);
    memcpy(ubo.projection, camera->mat_proj, sizeof(ubo.projection));

    memcpy(renderer->pipeline3D.mcpBuffersMapped[renderer->currentFrame], &ubo, sizeof(ubo));
}

void renderer_render(Renderer* renderer, World* world, Camera* camera, Picker* picker, LinkedList* panels) {
    vkWaitForFences(renderer->vulkan->device, 1, &renderer->inFlightFences[renderer->currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(renderer->vulkan->device, renderer->swapChain.swapChain, UINT64_MAX, renderer->imageAvailableSemaphores[renderer->currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        renderer_recreate_swap_chain(renderer);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        printf("failed to acquire swap chain image.\n");
        assert(false);
    }

    vkResetFences(renderer->vulkan->device, 1, &renderer->inFlightFences[renderer->currentFrame]);

    vkResetCommandBuffer(renderer->commandBuffers[renderer->currentFrame], 0);
    renderer_record_command_buffer(renderer, renderer->commandBuffers[renderer->currentFrame], imageIndex, world, camera, picker, panels);

    renderer_3D_update_uniforms(renderer, camera);

    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {renderer->imageAvailableSemaphores[renderer->currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderer->commandBuffers[renderer->currentFrame];
    VkSemaphore signalSemaphores[] = {renderer->renderFinishedSemaphores[renderer->currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(renderer->graphicsQueue, 1, &submitInfo, renderer->inFlightFences[renderer->currentFrame]) != VK_SUCCESS) {
        printf("failed to submit draw command buffer.\n");
        assert(false);
    }

    VkPresentInfoKHR presentInfo = { 0 };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {renderer->swapChain.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    result = vkQueuePresentKHR(renderer->presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || renderer->framebufferResized) {
        renderer->framebufferResized = false;
        renderer_recreate_swap_chain(renderer);
    } else if (result != VK_SUCCESS) {
        printf("failed to present swap chain image.\n");
        assert(false);
    }

    renderer->currentFrame = (renderer->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void renderer_record_command_buffer(Renderer* renderer, VkCommandBuffer commandBuffer, uint32_t imageIndex, World* world, Camera* camera, Picker* picker, LinkedList* panels) {
    VkCommandBufferBeginInfo beginInfo = { 0 };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = NULL;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        printf("failed to begin recording command buffer.\n");
        assert(false);
    }

    VkClearValue clearValues[2] = { 0 };
    clearValues[0].color.float32[0] = 0.50f;
    clearValues[0].color.float32[1] = 0.75f;
    clearValues[0].color.float32[2] = 0.86f;
    clearValues[0].color.float32[3] = 1.0f;
    clearValues[1].depthStencil.depth = 1.0f;
    clearValues[1].depthStencil.stencil = 0.0f;

    VkRenderPassBeginInfo renderPassInfo = { 0 };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderer->renderPass;
    renderPassInfo.framebuffer = renderer->swapChain.frameBuffers[imageIndex];
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = renderer->swapChain.extent;
    renderPassInfo.clearValueCount =(uint32_t)(sizeof(clearValues) / sizeof(clearValues[0]));
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = { 0 };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)renderer->swapChain.extent.width;
    viewport.height = (float)renderer->swapChain.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = { 0 };
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = renderer->swapChain.extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    renderer_3D_record(renderer, world, camera, picker);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline2D.pipeline.pipeline);

    renderer_2D_record(renderer, panels);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        printf("failed to record command buffer.\n");
        assert(false);
    }
}

void renderer_3D_record_mesh(Renderer* renderer, Mesh* mesh, char mode) {
    VkBuffer vertexBuffers[] = { mesh->vbo };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(renderer->commandBuffers[renderer->currentFrame], 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(renderer->commandBuffers[renderer->currentFrame], mesh->ebo, 0, VK_INDEX_TYPE_UINT16);
    int elementsPerQuad = (mode == MESH_FILL) ? 4 : 5;
    for (int i = 0; i < mesh->quads.size; i++) {
        vkCmdDrawIndexed(renderer->commandBuffers[renderer->currentFrame], elementsPerQuad, 1, elementsPerQuad * i, 0, 0);
    }
}

void renderer_3D_record_ground(Renderer* renderer, Ground* ground, Camera* camera) {

    float worldPosition[] = {
        camera->mat_view[12],
        0,
        camera->mat_view[14],
    };
    float color[] = {
        192.0f/255,
        192.0f/255,
        192.0f/255
    };
    // World position
    vkCmdPushConstants(renderer->commandBuffers[renderer->currentFrame], renderer->pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 64, sizeof(worldPosition), worldPosition);
    // Color
    vkCmdPushConstants(renderer->commandBuffers[renderer->currentFrame], renderer->pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 80, sizeof(color), color);

    renderer_3D_record_mesh(renderer, &ground->mesh, MESH_FILL);
}

void renderer_3D_record(Renderer* renderer, World* world, Camera* camera, Picker* picker) {
    // Model matrix
    float mat4[16];
    mat4_identity(mat4);
    vkCmdPushConstants(renderer->commandBuffers[renderer->currentFrame], renderer->pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(mat4), mat4);

    // Sun position
    float vec3[] = { 100, 100, 100 };
    vkCmdPushConstants(renderer->commandBuffers[renderer->currentFrame], renderer->pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 96, sizeof(vec3), vec3);

    // Ambient
    float ambient = 0.2;
    vkCmdPushConstants(renderer->commandBuffers[renderer->currentFrame], renderer->pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 112, sizeof(ambient), &ambient);

    vkCmdBindPipeline(renderer->commandBuffers[renderer->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline3D.pipeline.pipeline);
    vkCmdBindDescriptorSets(renderer->commandBuffers[renderer->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline3D.pipeline.layout, 0, 1, &renderer->pipeline3D.descriptorSets[renderer->currentFrame], 0, NULL);

    renderer_3D_record_ground(renderer, &world->ground, camera);

    linked_list_foreach(&world->chunks, record_world_chunk, renderer);

    renderer_3D_record_picker(renderer, picker);
}

void renderer_2D_record(Renderer* renderer, LinkedList* panels) {
    // Projection
    int width = 0, height = 0;
    glfwGetFramebufferSize(renderer->window->glfwWindow, &width, &height);

    float clipCorrect[16];
    vulkan_clip_correction_matrix(clipCorrect);

    float mat[16];
    mat4_orthographic(mat, 0, width, 0, height);
    mat4_multiply(mat, clipCorrect, mat);
    vkCmdPushConstants(renderer->commandBuffers[renderer->currentFrame], renderer->pipeline2D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(mat), mat);

    linked_list_foreach(panels, record_panel, renderer);
}

void renderer_create_descriptor_sets(Renderer* renderer, VkImageView imageView, VkDescriptorSet** descriptorSets) {
    vulkan_create_descriptor_sets(renderer->vulkan->device, renderer->descriptorPool, MAX_FRAMES_IN_FLIGHT, renderer->pipeline2D.pipeline.descriptorSetLayout, descriptorSets);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorImageInfo imageInfo = { 0 };
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imageView;
        imageInfo.sampler = renderer->pipeline2D.sampler;

        VkWriteDescriptorSet descriptorWrite = { 0 };
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = (*descriptorSets)[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = NULL;
        descriptorWrite.pImageInfo = &imageInfo;
        descriptorWrite.pTexelBufferView = NULL;

        vkUpdateDescriptorSets(renderer->vulkan->device, 1, &descriptorWrite, 0, NULL);
    }
}
