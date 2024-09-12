#include "renderer.h"
#include "internal/renderer.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

/* OpenGL */

void render_mesh(void* ptr, void* rendererPtr) {
    Mesh* mesh = (Mesh*)ptr;
    Renderer* renderer = (Renderer*)rendererPtr;

    float color[3];
    block_color_rgb(mesh->color, color);
    renderer_3D_update_color(renderer, color[0], color[1], color[2]);
    renderer_render_mesh(renderer, mesh, MESH_FILL);
}

void render_world_chunk(void* worldChunkPtr, void* rendererPtr) {
    WorldChunk* worldChunk = (WorldChunk*)worldChunkPtr;
    Renderer* renderer = (Renderer*)rendererPtr;
    float position[3] = {
        worldChunk->id.x * WORLD_CHUNK_LENGTH,
        worldChunk->id.y * WORLD_CHUNK_LENGTH,
        worldChunk->id.z * WORLD_CHUNK_LENGTH
    };

    if (worldChunk->chunk->meshes.size)
        renderer_render_chunk(renderer, worldChunk->chunk, position);
}

void render_panel(void* panelPtr, void* rendererPtr) {
    Panel* panel = (Panel*)panelPtr;
    Renderer* renderer = (Renderer*)rendererPtr;
    renderer_render_panel(renderer, panel);
}

void renderer_render_panel(Renderer* renderer, Panel* panel) {
    renderer_2D_use(renderer);
    glBindBuffer(GL_ARRAY_BUFFER, panel->renderState.opengl.vbo);

    glEnableVertexAttribArray(renderer->renderState.opengl.shaderProgram2D.attrib_position);
    glVertexAttribPointer(renderer->renderState.opengl.shaderProgram2D.attrib_position, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
    glEnableVertexAttribArray(renderer->renderState.opengl.shaderProgram2D.attrib_texcoord);
    glVertexAttribPointer(renderer->renderState.opengl.shaderProgram2D.attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));

    panel->drawCallback(panel->owner);
    panel_texture(panel);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/* Vulkan */

void record_mesh(void* ptr, void* rendererPtr) {
    Mesh* mesh = (Mesh*)ptr;
    Renderer* renderer = (Renderer*)rendererPtr;

    float color[3];
    block_color_rgb(mesh->color, color);
    color[0] /= 255.0f;
    color[1] /= 255.0f;
    color[2] /= 255.0f;

    // Color
    PFN_vkCmdPushConstants pfnCmdPushConstants =
        (PFN_vkCmdPushConstants)glfwGetInstanceProcAddress(NULL, "vkCmdPushConstants");

    pfnCmdPushConstants(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], renderer->renderState.vulkan.pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 80, sizeof(color), color);

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
    VkBuffer vertexBuffers[] = { panel->renderState.vulkan.vbo };
    VkDeviceSize offsets[] = { 0 };

    PFN_vkCmdBindDescriptorSets pfnCmdBindDescriptorSets =
        (PFN_vkCmdBindDescriptorSets)glfwGetInstanceProcAddress(NULL, "vkCmdBindDescriptorSets");
    PFN_vkCmdBindVertexBuffers pfnCmdBindVertexBuffers =
        (PFN_vkCmdBindVertexBuffers)glfwGetInstanceProcAddress(NULL, "vkCmdBindVertexBuffers");
    PFN_vkCmdDraw pfnCmdDraw =
        (PFN_vkCmdDraw)glfwGetInstanceProcAddress(NULL, "vkCmdDraw");

    pfnCmdBindDescriptorSets(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->renderState.vulkan.pipeline2D.pipeline.layout, 0, 1, &panel->renderState.vulkan.descriptorSets[renderer->renderState.vulkan.currentFrame], 0, NULL);
    pfnCmdBindVertexBuffers(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], 0, 1, vertexBuffers, offsets);
    pfnCmdDraw(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], 4, 1, 0, 0);
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

    PFN_vkCreateDescriptorSetLayout pfnCreateDescriptorSetLayout =
        (PFN_vkCreateDescriptorSetLayout)glfwGetInstanceProcAddress(NULL, "vkCreateDescriptorSetLayout");

    if (pfnCreateDescriptorSetLayout(renderer->vulkan->device, &layoutInfo, NULL, dsLayout) != VK_SUCCESS) {
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
                        renderer->renderState.vulkan.swapChain.extent.width,
                        renderer->renderState.vulkan.swapChain.extent.height,
                        depthFormat,
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        &renderer->renderState.vulkan.depthImage,
                        &renderer->renderState.vulkan.depthImageMemory);
    renderer->renderState.vulkan.depthImageView = vulkan_create_image_view(renderer->vulkan->device, renderer->renderState.vulkan.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

}

void renderer_setup_framebuffers(Renderer* renderer) {
    for (int i = 0; i < renderer->renderState.vulkan.swapChain.imageCount; i++) {
        VkImageView attachments[] = { renderer->renderState.vulkan.swapChain.imageViews[i], renderer->renderState.vulkan.depthImageView };

        VkFramebufferCreateInfo frameBufferInfo = { 0 };
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = renderer->renderState.vulkan.renderPass;
        frameBufferInfo.attachmentCount = (uint32_t)(sizeof(attachments)/sizeof(attachments[0]));
        frameBufferInfo.pAttachments = attachments;
        frameBufferInfo.width = renderer->renderState.vulkan.swapChain.extent.width;
        frameBufferInfo.height = renderer->renderState.vulkan.swapChain.extent.height;
        frameBufferInfo.layers = 1;

        PFN_vkCreateFramebuffer pfnCreateFramebuffer =
            (PFN_vkCreateFramebuffer)glfwGetInstanceProcAddress(NULL, "vkCreateFramebuffer");

        if (pfnCreateFramebuffer(renderer->vulkan->device, &frameBufferInfo, NULL, &renderer->renderState.vulkan.swapChain.frameBuffers[i]) != VK_SUCCESS) {
            printf("failed to create framebuffer.\n");
            assert(false);
        }
    }
}

Renderer* renderer_init(Renderer* r, Window* window, Vulkan* vulkan) {
    Renderer* renderer = r ? r : NEW(Renderer, 1);

    renderer->window = window;
    renderer->vulkan = vulkan;

    if (!renderer->vulkan) {
        shader_program_3D_init(&renderer->renderState.opengl.shaderProgram3D);

        float mat4[16];
        renderer_3D_update_model(renderer, mat4_identity(mat4));
        renderer_3D_update_ambient(renderer, 0.4);

        float sunPosition[] = { 100, 100, 100 };
        renderer_3D_update_sun_position(renderer, sunPosition);

        shader_program_2D_init(&renderer->renderState.opengl.shaderProgram2D);

        glActiveTexture(GL_TEXTURE0);
        renderer_2D_update_sampler(renderer, 0);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glClearColor(0.50f, 0.75f, 0.86f, 1.0f);
        glClearDepthf(1);
        glEnable( GL_DEPTH_TEST );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glLineWidth(2);
    } else {
        vulkan_create_swap_chain(vulkan->physicalDevice, vulkan->device, window->glfwWindow, window->surface, &renderer->renderState.vulkan.swapChain);

        vulkan_create_render_pass(vulkan->physicalDevice, vulkan->device, &renderer->renderState.vulkan.swapChain, &renderer->renderState.vulkan.renderPass);

        renderer_3D_create_pipeline(renderer);
        renderer_2D_create_pipeline(renderer);

        renderer_setup_depth_resources(renderer);

        renderer_setup_framebuffers(renderer);

        vulkan_create_sampler(vulkan->physicalDevice, vulkan->device, &renderer->renderState.vulkan.pipeline2D.sampler);

        renderer_3D_create_uniform_buffers(renderer);

        renderer_create_descriptor_pool(renderer);

        renderer_3D_create_descriptor_sets(renderer);


        renderer_create_command_buffers(renderer);

        renderer_create_sync_objects(renderer);

        renderer->renderState.vulkan.currentFrame = 0;
        renderer->renderState.vulkan.framebufferResized = false;
    }

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
    renderer_3D_create_descriptor_set_layout(renderer, &renderer->renderState.vulkan.pipeline3D.pipeline.descriptorSetLayout);
    renderer_3D_create_descriptor_set_layout(renderer, &renderer->renderState.vulkan.pipeline3D.pipelineLine.descriptorSetLayout);

    GBytes* _3D_vert_spv = g_resources_lookup_data("/src/shaders/spir-v/3D.vert.spv", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
    GBytes* _3D_frag_spv = g_resources_lookup_data("/src/shaders/spir-v/3D.frag.spv", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);

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
                           renderer->renderState.vulkan.renderPass,
                           VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                           &renderer->renderState.vulkan.pipeline3D.pipeline);
    vulkan_create_pipeline(renderer->vulkan->device,
                           &vertShaderStageInfo,
                           &fragShaderStageInfo,
                           &bindingDescription,
                           attributeDescriptions,
                           attributeDescriptionsCount,
                           &pushConstants,
                           renderer->renderState.vulkan.renderPass,
                           VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
                           &renderer->renderState.vulkan.pipeline3D.pipelineLine);

    PFN_vkDestroyShaderModule pfnDestroyShaderModule =
        (PFN_vkDestroyShaderModule)glfwGetInstanceProcAddress(NULL, "vkDestroyShaderModule");

    pfnDestroyShaderModule(renderer->vulkan->device, fragShaderModule, NULL);
    pfnDestroyShaderModule(renderer->vulkan->device, vertShaderModule, NULL);

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

    PFN_vkCreateDescriptorSetLayout pfnCreateDescriptorSetLayout =
        (PFN_vkCreateDescriptorSetLayout)glfwGetInstanceProcAddress(NULL, "vkCreateDescriptorSetLayout");

    if (pfnCreateDescriptorSetLayout(renderer->vulkan->device, &layoutInfo, NULL, dsLayout) != VK_SUCCESS) {
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
    renderer_2D_create_descriptor_set_layout(renderer, &renderer->renderState.vulkan.pipeline2D.pipeline.descriptorSetLayout);

    GBytes* _2D_vert_spv = g_resources_lookup_data("/src/shaders/spir-v/2D.vert.spv", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
    GBytes* _2D_frag_spv = g_resources_lookup_data("/src/shaders/spir-v/2D.frag.spv", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);

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
                           renderer->renderState.vulkan.renderPass,
                           VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                           &renderer->renderState.vulkan.pipeline2D.pipeline);

    PFN_vkDestroyShaderModule pfnDestroyShaderModule =
        (PFN_vkDestroyShaderModule)glfwGetInstanceProcAddress(NULL, "vkDestroyShaderModule");

    pfnDestroyShaderModule(renderer->vulkan->device, fragShaderModule, NULL);
    pfnDestroyShaderModule(renderer->vulkan->device, vertShaderModule, NULL);

    free(attributeDescriptions);
}

void renderer_destroy(Renderer* renderer) {
    if (!renderer->vulkan) {
        shader_program_2D_destroy(&renderer->renderState.opengl.shaderProgram2D);
        shader_program_3D_destroy(&renderer->renderState.opengl.shaderProgram3D);
    } else {
        PFN_vkDestroySampler pfnDestroySampler =
            (PFN_vkDestroySampler)glfwGetInstanceProcAddress(NULL, "vkDestroySampler");
        PFN_vkUnmapMemory pfnUnmapMemory =
            (PFN_vkUnmapMemory)glfwGetInstanceProcAddress(NULL, "vkUnmapMemory");
        PFN_vkDestroyBuffer pfnDestroyBuffer =
            (PFN_vkDestroyBuffer)glfwGetInstanceProcAddress(NULL, "vkDestroyBuffer");
        PFN_vkFreeMemory pfnFreeMemory =
            (PFN_vkFreeMemory)glfwGetInstanceProcAddress(NULL, "vkFreeMemory");

        pfnDestroySampler(renderer->vulkan->device, renderer->renderState.vulkan.pipeline2D.sampler, NULL);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            pfnUnmapMemory(renderer->vulkan->device, renderer->renderState.vulkan.pipeline3D.mcpBuffersMemory[i]);
            pfnDestroyBuffer(renderer->vulkan->device, renderer->renderState.vulkan.pipeline3D.mcpBuffers[i], NULL);
            pfnFreeMemory(renderer->vulkan->device, renderer->renderState.vulkan.pipeline3D.mcpBuffersMemory[i], NULL);
        }
        free(renderer->renderState.vulkan.pipeline3D.descriptorSets);

        PFN_vkFreeCommandBuffers pfnFreeCommandBuffers =
            (PFN_vkFreeCommandBuffers)glfwGetInstanceProcAddress(NULL, "vkFreeCommandBuffers");

        pfnFreeCommandBuffers(renderer->vulkan->device, renderer->vulkan->commandPool, MAX_FRAMES_IN_FLIGHT, renderer->renderState.vulkan.commandBuffers);
        free(renderer->renderState.vulkan.commandBuffers);

        PFN_vkDestroyDescriptorPool pfnDestroyDescriptorPool =
            (PFN_vkDestroyDescriptorPool)glfwGetInstanceProcAddress(NULL, "vkDestroyDescriptorPool");
        PFN_vkDestroyDescriptorSetLayout pfnDestroyDescriptorSetLayout =
            (PFN_vkDestroyDescriptorSetLayout)glfwGetInstanceProcAddress(NULL, "vkDestroyDescriptorSetLayout");

        pfnDestroyDescriptorPool(renderer->vulkan->device, renderer->renderState.vulkan.descriptorPool, NULL);
        pfnDestroyDescriptorSetLayout(renderer->vulkan->device, renderer->renderState.vulkan.pipeline3D.pipeline.descriptorSetLayout, NULL);
        pfnDestroyDescriptorSetLayout(renderer->vulkan->device, renderer->renderState.vulkan.pipeline3D.pipelineLine.descriptorSetLayout, NULL);
        pfnDestroyDescriptorSetLayout(renderer->vulkan->device, renderer->renderState.vulkan.pipeline2D.pipeline.descriptorSetLayout, NULL);

        PFN_vkDestroyPipeline pfnDestroyPipeline =
            (PFN_vkDestroyPipeline)glfwGetInstanceProcAddress(NULL, "vkDestroyPipeline");
        PFN_vkDestroyPipelineLayout pfnDestroyPipelineLayout =
            (PFN_vkDestroyPipelineLayout)glfwGetInstanceProcAddress(NULL, "vkDestroyPipelineLayout");

        pfnDestroyPipeline(renderer->vulkan->device, renderer->renderState.vulkan.pipeline2D.pipeline.pipeline, NULL);
        pfnDestroyPipelineLayout(renderer->vulkan->device, renderer->renderState.vulkan.pipeline2D.pipeline.layout, NULL);
        pfnDestroyPipeline(renderer->vulkan->device, renderer->renderState.vulkan.pipeline3D.pipeline.pipeline, NULL);
        pfnDestroyPipelineLayout(renderer->vulkan->device, renderer->renderState.vulkan.pipeline3D.pipeline.layout, NULL);
        pfnDestroyPipeline(renderer->vulkan->device, renderer->renderState.vulkan.pipeline3D.pipelineLine.pipeline, NULL);
        pfnDestroyPipelineLayout(renderer->vulkan->device, renderer->renderState.vulkan.pipeline3D.pipelineLine.layout, NULL);

        PFN_vkDestroyRenderPass pfnDestroyRenderPass =
            (PFN_vkDestroyRenderPass)glfwGetInstanceProcAddress(NULL, "vkDestroyRenderPass");

        pfnDestroyRenderPass(renderer->vulkan->device, renderer->renderState.vulkan.renderPass, NULL);

        renderer_cleanup_swap_chain(renderer);

        PFN_vkDestroySemaphore pfnDestroySemaphore =
            (PFN_vkDestroySemaphore)glfwGetInstanceProcAddress(NULL, "vkDestroySemaphore");
        PFN_vkDestroyFence pfnDestroyFence =
            (PFN_vkDestroyFence)glfwGetInstanceProcAddress(NULL, "vkDestroyFence");

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            pfnDestroySemaphore(renderer->vulkan->device, renderer->renderState.vulkan.imageAvailableSemaphores[i], NULL);
            pfnDestroySemaphore(renderer->vulkan->device, renderer->renderState.vulkan.renderFinishedSemaphores[i], NULL);
            pfnDestroyFence(renderer->vulkan->device, renderer->renderState.vulkan.inFlightFences[i], NULL);
        }
    }
}

void renderer_clear(Renderer* renderer) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_resize(Renderer* renderer, int width, int height, Camera* camera) {
#ifndef _WIN32
    const char* sessionType = getenv("XDG_SESSION_TYPE");
    if (sessionType && strcmp(sessionType, "x11") == 0) {
        glViewport(0, 0, width, height);

        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        float xscale, yscale;
        glfwGetMonitorContentScale(primary, &xscale, &yscale);

        float mat[16];
        mat4_orthographic(mat, 0, width / xscale, 0, height / yscale);
        renderer_2D_update_projection(renderer, mat);
        camera_set_aspect(camera, (float)width / height);
        renderer_apply_camera(renderer, camera);
    } else {
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        float xscale, yscale;
        glfwGetMonitorContentScale(primary, &xscale, &yscale);

        glViewport(0, 0, width * xscale, height * yscale);

        float mat[16];
        mat4_orthographic(mat, 0, width, 0, height);
        renderer_2D_update_projection(renderer, mat);
        camera_set_aspect(camera, (float)width / height);
        renderer_apply_camera(renderer, camera);
    }
#else
    glViewport(0, 0, width, height);

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(primary, &xscale, &yscale);

    float mat[16];
    mat4_orthographic(mat, 0, width / xscale, 0, height / yscale);
    renderer_2D_update_projection(renderer, mat);
    camera_set_aspect(camera, (float)width / height);
    renderer_apply_camera(renderer, camera);
#endif
}

void renderer_3D_update_world_position(Renderer* renderer, float* position) {
    shader_program_3D_use(&renderer->renderState.opengl.shaderProgram3D);
    shader_program_3D_update_world_position(&renderer->renderState.opengl.shaderProgram3D, position);
}

void renderer_3D_update_model(Renderer* renderer, float* mat4) {
    shader_program_3D_use(&renderer->renderState.opengl.shaderProgram3D);
    shader_program_3D_update_model(&renderer->renderState.opengl.shaderProgram3D, mat4);
}

void renderer_3D_update_camera(Renderer* renderer, float* mat4) {
    shader_program_3D_use(&renderer->renderState.opengl.shaderProgram3D);
    shader_program_3D_update_camera(&renderer->renderState.opengl.shaderProgram3D, mat4);
}

void renderer_3D_update_projection(Renderer* renderer, float* mat4) {
    shader_program_3D_use(&renderer->renderState.opengl.shaderProgram3D);
    shader_program_3D_update_projection(&renderer->renderState.opengl.shaderProgram3D, mat4);
}

void renderer_3D_update_color(Renderer* renderer, float r, float g, float b) {
    shader_program_3D_use(&renderer->renderState.opengl.shaderProgram3D);
    shader_program_3D_update_color(&renderer->renderState.opengl.shaderProgram3D, r, g, b);
}

void renderer_3D_update_ambient(Renderer* renderer, float a) {
    shader_program_3D_use(&renderer->renderState.opengl.shaderProgram3D);
    shader_program_3D_update_ambient(&renderer->renderState.opengl.shaderProgram3D, a);
}

void renderer_3D_update_sun_position(Renderer* renderer, float* position) {
    shader_program_3D_use(&renderer->renderState.opengl.shaderProgram3D);
    shader_program_3D_update_sun_position(&renderer->renderState.opengl.shaderProgram3D, position);
}

void renderer_apply_camera(Renderer* renderer, Camera* camera) {
    mat4_perspective(camera->mat_proj, camera->fov, camera->aspect, camera->_near, camera->_far);

    if (!renderer->vulkan) {
        float mat[16];
        mat4_multiply(mat, camera->mat_view, camera->mat_model);
        mat4_inverse(mat, mat);
        renderer_3D_update_camera(renderer, mat);

        renderer_3D_update_projection(renderer, camera->mat_proj);
        mat4_inverse(camera->mat_proj_inv, camera->mat_proj);
    } else {
        float clipCorrect[16];
        vulkan_clip_correction_matrix(clipCorrect);
        mat4_multiply(camera->mat_proj, clipCorrect, camera->mat_proj);
        renderer_3D_update_uniforms(renderer, camera);
        mat4_inverse(camera->mat_proj_inv, camera->mat_proj);
    }
}

void renderer_3D_use(Renderer* renderer) {
    shader_program_3D_use(&renderer->renderState.opengl.shaderProgram3D);
}

void renderer_2D_update_projection(Renderer* renderer, float* mat4) {
    shader_program_2D_use(&renderer->renderState.opengl.shaderProgram2D);
    shader_program_2D_update_projection(&renderer->renderState.opengl.shaderProgram2D, mat4);
}

void renderer_2D_update_sampler(Renderer* renderer, GLint sampler) {
    shader_program_2D_use(&renderer->renderState.opengl.shaderProgram2D);
    shader_program_2D_update_sampler(&renderer->renderState.opengl.shaderProgram2D, sampler);
}

void renderer_2D_use(Renderer* renderer) {
    shader_program_2D_use(&renderer->renderState.opengl.shaderProgram2D);
}

void renderer_render_chunk(Renderer* renderer, Chunk* chunk, float* position) {
    renderer_3D_update_world_position(renderer, position);
    linked_list_foreach(&chunk->meshes, render_mesh, renderer);
}

void renderer_render_mesh(Renderer* renderer, Mesh* mesh, char mode) {
    renderer_3D_use(renderer);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->renderState.opengl.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->renderState.opengl.ebo);

    glEnableVertexAttribArray(renderer->renderState.opengl.shaderProgram3D.attrib_position);
    glVertexAttribPointer(renderer->renderState.opengl.shaderProgram3D.attrib_position, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);
    glEnableVertexAttribArray(renderer->renderState.opengl.shaderProgram3D.attrib_normal);
    glVertexAttribPointer(renderer->renderState.opengl.shaderProgram3D.attrib_normal, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));

    for (int q=0; q<mesh->quads.size; q++)
        glDrawElements(mode == MESH_FILL ? GL_TRIANGLE_STRIP : GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*) (4*q*sizeof(GLushort)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void renderer_render_ground(Renderer* renderer, Ground* ground, Camera* camera) {
    float worldPosition[] = {
        camera->mat_view[12],
        0,
        camera->mat_view[14],
    };
    renderer_3D_update_world_position(renderer, worldPosition);
    renderer_3D_update_color(renderer, 192, 192, 192);
    renderer_render_mesh(renderer, &ground->mesh, MESH_FILL);
}

void renderer_render_world(Renderer* renderer, World* world, Camera* camera) {
    renderer_render_ground(renderer, &world->ground, camera);

    linked_list_foreach(&world->chunks, render_world_chunk, renderer);
}

void renderer_render_picker(Renderer* renderer, Picker* picker) {
    float mat[16];
    float vec[3];

    if (picker->selection.present) {
        renderer_3D_update_world_position(renderer, picker->selection.box.position);
        renderer_3D_update_color(renderer, 0,255,255);
        renderer_render_mesh(renderer, &picker->selection.mesh, MESH_LINE);
    }

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
        renderer_3D_update_model(renderer, mat);
        renderer_render_chunk(renderer, picker->selection.model, picker->box.position);
    }

    mat4_identity(mat);
    renderer_3D_update_model(renderer, mat);

    renderer_3D_update_world_position(renderer, picker->box.position);
    renderer_3D_update_color(renderer, 255,255,0);
    renderer_render_mesh(renderer, &picker->mesh, MESH_LINE);
}

void renderer_render_panels(Renderer* renderer, LinkedList* panels) {
    linked_list_foreach(panels, render_panel, renderer);
}

void renderer_vulkan_resize(Renderer* renderer) {
    renderer->renderState.vulkan.framebufferResized = true;
}

void renderer_3D_record_chunk(Renderer* renderer, Chunk* chunk, float* position) {
    // World position
    PFN_vkCmdPushConstants pfnCmdPushConstants =
        (PFN_vkCmdPushConstants)glfwGetInstanceProcAddress(NULL, "vkCmdPushConstants");

    pfnCmdPushConstants(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], renderer->renderState.vulkan.pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 64, sizeof(float[3]), position);

    linked_list_foreach(&chunk->meshes, record_mesh, renderer);
}

void renderer_3D_record_picker(Renderer* renderer, Picker* picker) {
    float mat[16];
    float vec[3];

    PFN_vkCmdPushConstants pfnCmdPushConstants =
        (PFN_vkCmdPushConstants)glfwGetInstanceProcAddress(NULL, "vkCmdPushConstants");

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
        pfnCmdPushConstants(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], renderer->renderState.vulkan.pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(mat), mat);

        renderer_3D_record_chunk(renderer, picker->selection.model, picker->box.position);
    }

    PFN_vkCmdBindPipeline pfnCmdBindPipeline =
        (PFN_vkCmdBindPipeline)glfwGetInstanceProcAddress(NULL, "vkCmdBindPipeline");
    PFN_vkCmdBindDescriptorSets pfnCmdBindDescriptorSets =
        (PFN_vkCmdBindDescriptorSets)glfwGetInstanceProcAddress(NULL, "vkCmdBindDescriptorSets");

    pfnCmdBindPipeline(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->renderState.vulkan.pipeline3D.pipelineLine.pipeline);
    pfnCmdBindDescriptorSets(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->renderState.vulkan.pipeline3D.pipelineLine.layout, 0, 1, &renderer->renderState.vulkan.pipeline3D.descriptorSets[renderer->renderState.vulkan.currentFrame], 0, NULL);

    if (picker->selection.present) {
        float color[] = { 0, 1, 1 };

        // World position
        pfnCmdPushConstants(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], renderer->renderState.vulkan.pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 64, sizeof(picker->selection.box.position),  picker->selection.box.position);
        // Color
        pfnCmdPushConstants(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], renderer->renderState.vulkan.pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 80, sizeof(color), color);

        renderer_3D_record_mesh(renderer, &picker->selection.mesh, MESH_LINE);
    }

    mat4_identity(mat);
    // Model matrix
    pfnCmdPushConstants(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], renderer->renderState.vulkan.pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(mat), mat);
    // World position
    pfnCmdPushConstants(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], renderer->renderState.vulkan.pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 64, sizeof(picker->box.position),  picker->box.position);
    // Color
    float color[] = { 1, 1, 0 };
    pfnCmdPushConstants(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], renderer->renderState.vulkan.pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 80, sizeof(color), color);

    renderer_3D_record_mesh(renderer, &picker->mesh, MESH_LINE);
}

void renderer_record_panels(Renderer* renderer, LinkedList* panels) {
    linked_list_foreach(panels, record_panel, renderer);
}

void renderer_3D_create_uniform_buffers(Renderer* renderer) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject3D);

    renderer->renderState.vulkan.pipeline3D.mcpBuffers = NEW(VkBuffer, MAX_FRAMES_IN_FLIGHT);
    renderer->renderState.vulkan.pipeline3D.mcpBuffersMemory = NEW(VkDeviceMemory, MAX_FRAMES_IN_FLIGHT);
    renderer->renderState.vulkan.pipeline3D.mcpBuffersMapped = NEW(void*, MAX_FRAMES_IN_FLIGHT);

    PFN_vkMapMemory pfnMapMemory =
        (PFN_vkMapMemory)glfwGetInstanceProcAddress(NULL, "vkMapMemory");

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vulkan_create_buffer(renderer->vulkan->physicalDevice,
                             renderer->vulkan->device, bufferSize,
                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             &renderer->renderState.vulkan.pipeline3D.mcpBuffers[i],
                             &renderer->renderState.vulkan.pipeline3D.mcpBuffersMemory[i]);
        pfnMapMemory(renderer->vulkan->device, renderer->renderState.vulkan.pipeline3D.mcpBuffersMemory[i], 0, bufferSize, 0, &renderer->renderState.vulkan.pipeline3D.mcpBuffersMapped[i]);
    }
}

void renderer_create_descriptor_pool(Renderer* renderer) {
    VkDescriptorPoolSize poolSizes[2] = { 0 };
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (uint32_t)(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = (uint32_t)(8);

    vulkan_create_descriptor_set_pool(renderer->vulkan->device, poolSizes, sizeof(poolSizes)/sizeof(poolSizes[0]), &renderer->renderState.vulkan.descriptorPool, 8);
}

void renderer_3D_create_descriptor_sets(Renderer* renderer) {
    vulkan_create_descriptor_sets(renderer->vulkan->device, renderer->renderState.vulkan.descriptorPool, MAX_FRAMES_IN_FLIGHT, renderer->renderState.vulkan.pipeline3D.pipeline.descriptorSetLayout, &renderer->renderState.vulkan.pipeline3D.descriptorSets);

    PFN_vkUpdateDescriptorSets pfnUpdateDescriptorSets =
        (PFN_vkUpdateDescriptorSets)glfwGetInstanceProcAddress(NULL, "vkUpdateDescriptorSets");

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo = { 0 };
        bufferInfo.buffer = renderer->renderState.vulkan.pipeline3D.mcpBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject3D);

        VkWriteDescriptorSet descriptorWrite = { 0 };
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = renderer->renderState.vulkan.pipeline3D.descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = NULL;
        descriptorWrite.pTexelBufferView = NULL;

        pfnUpdateDescriptorSets(renderer->vulkan->device, 1, &descriptorWrite, 0, NULL);
    }
}

void renderer_create_command_buffers(Renderer* renderer) {
    renderer->renderState.vulkan.commandBuffers = NEW(VkCommandBuffer, MAX_FRAMES_IN_FLIGHT);
    vulkan_create_command_buffers(renderer->vulkan->device, renderer->vulkan->commandPool, MAX_FRAMES_IN_FLIGHT, renderer->renderState.vulkan.commandBuffers);
}

void renderer_create_sync_objects(Renderer* renderer) {
    renderer->renderState.vulkan.imageAvailableSemaphores = NEW(VkSemaphore, MAX_FRAMES_IN_FLIGHT);
    renderer->renderState.vulkan.renderFinishedSemaphores = NEW(VkSemaphore, MAX_FRAMES_IN_FLIGHT);
    renderer->renderState.vulkan.inFlightFences = NEW(VkFence, MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = { 0 };
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = { 0 };
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    PFN_vkCreateSemaphore pfnCreateSemaphore =
        (PFN_vkCreateSemaphore)glfwGetInstanceProcAddress(NULL, "vkCreateSemaphore");
    PFN_vkCreateFence pfnCreateFence =
        (PFN_vkCreateFence)glfwGetInstanceProcAddress(NULL, "vkCreateFence");

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (pfnCreateSemaphore(renderer->vulkan->device, &semaphoreInfo, NULL, &renderer->renderState.vulkan.imageAvailableSemaphores[i]) != VK_SUCCESS ||
                pfnCreateSemaphore(renderer->vulkan->device, &semaphoreInfo, NULL, &renderer->renderState.vulkan.renderFinishedSemaphores[i]) != VK_SUCCESS ||
                pfnCreateFence(renderer->vulkan->device, &fenceInfo, NULL, &renderer->renderState.vulkan.inFlightFences[i]) != VK_SUCCESS) {
            printf("failed to create sync objects.\n");
            assert(false);
        }
    }
}

void renderer_cleanup_swap_chain(Renderer* renderer) {
    PFN_vkDestroyImageView pfnDestroyImageView =
        (PFN_vkDestroyImageView)glfwGetInstanceProcAddress(NULL, "vkDestroyImageView");
    PFN_vkDestroyImage pfnDestroyImage =
        (PFN_vkDestroyImage)glfwGetInstanceProcAddress(NULL, "vkDestroyImage");
    PFN_vkFreeMemory pfnFreeMemory =
        (PFN_vkFreeMemory)glfwGetInstanceProcAddress(NULL, "vkFreeMemory");

    PFN_vkDestroyFramebuffer pfnDestroyFramebuffer =
        (PFN_vkDestroyFramebuffer)glfwGetInstanceProcAddress(NULL, "vkDestroyFramebuffer");
    PFN_vkDestroySwapchainKHR pfnDestroySwapchainKHR =
        (PFN_vkDestroySwapchainKHR)glfwGetInstanceProcAddress(NULL, "vkDestroySwapchainKHR");

    pfnDestroyImageView(renderer->vulkan->device, renderer->renderState.vulkan.depthImageView, NULL);
    pfnDestroyImage(renderer->vulkan->device, renderer->renderState.vulkan.depthImage, NULL);
    pfnFreeMemory(renderer->vulkan->device, renderer->renderState.vulkan.depthImageMemory, NULL);
    for (int i = 0; i < renderer->renderState.vulkan.swapChain.imageCount; i++) {
        pfnDestroyFramebuffer(renderer->vulkan->device, renderer->renderState.vulkan.swapChain.frameBuffers[i], NULL);
    }
    for (int i = 0; i < renderer->renderState.vulkan.swapChain.imageCount; i++) {
        pfnDestroyImageView(renderer->vulkan->device, renderer->renderState.vulkan.swapChain.imageViews[i], NULL);
    }
    free(renderer->renderState.vulkan.swapChain.frameBuffers);
    pfnDestroySwapchainKHR(renderer->vulkan->device, renderer->renderState.vulkan.swapChain.swapChain, NULL);
}

void renderer_recreate_swap_chain(Renderer* renderer) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(renderer->window->glfwWindow, &width, &height);
    while (width == 0 && height == 0) {
        glfwGetFramebufferSize(renderer->window->glfwWindow, &width, &height);
        glfwWaitEvents();
    }

    PFN_vkDeviceWaitIdle pfnDeviceWaitIdle =
        (PFN_vkDeviceWaitIdle)glfwGetInstanceProcAddress(NULL, "vkDeviceWaitIdle");

    pfnDeviceWaitIdle(renderer->vulkan->device);

    renderer_cleanup_swap_chain(renderer);

    vulkan_create_swap_chain(renderer->vulkan->physicalDevice, renderer->vulkan->device, renderer->window->glfwWindow, renderer->window->surface, &renderer->renderState.vulkan.swapChain);
    renderer_setup_depth_resources(renderer);
    renderer_setup_framebuffers(renderer);
}

void renderer_3D_update_uniforms(Renderer* renderer, Camera* camera) {
    UniformBufferObject3D ubo = { 0 };

    mat4_multiply(ubo.camera, camera->mat_view, camera->mat_model);
    mat4_inverse(ubo.camera, ubo.camera);
    memcpy(ubo.projection, camera->mat_proj, sizeof(ubo.projection));

    memcpy(renderer->renderState.vulkan.pipeline3D.mcpBuffersMapped[renderer->renderState.vulkan.currentFrame], &ubo, sizeof(ubo));
}

void renderer_vulkan_render(Renderer* renderer, World* world, Camera* camera, Picker* picker, LinkedList* panels) {
    PFN_vkWaitForFences pfnWaitForFences =
        (PFN_vkWaitForFences)glfwGetInstanceProcAddress(NULL, "vkWaitForFences");
    PFN_vkAcquireNextImageKHR pfnAcquireNextImageKHR =
        (PFN_vkAcquireNextImageKHR)glfwGetInstanceProcAddress(NULL, "vkAcquireNextImageKHR");

    pfnWaitForFences(renderer->vulkan->device, 1, &renderer->renderState.vulkan.inFlightFences[renderer->renderState.vulkan.currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = pfnAcquireNextImageKHR(renderer->vulkan->device, renderer->renderState.vulkan.swapChain.swapChain, UINT64_MAX, renderer->renderState.vulkan.imageAvailableSemaphores[renderer->renderState.vulkan.currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        renderer_recreate_swap_chain(renderer);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        printf("failed to acquire swap chain image.\n");
        assert(false);
    }

    PFN_vkResetFences pfnResetFences =
        (PFN_vkResetFences)glfwGetInstanceProcAddress(NULL, "vkResetFences");
    PFN_vkResetCommandBuffer pfnResetCommandBuffer =
        (PFN_vkResetCommandBuffer)glfwGetInstanceProcAddress(NULL, "vkResetCommandBuffer");

    pfnResetFences(renderer->vulkan->device, 1, &renderer->renderState.vulkan.inFlightFences[renderer->renderState.vulkan.currentFrame]);

    pfnResetCommandBuffer(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], 0);
    renderer_record_command_buffer(renderer, renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], imageIndex, world, camera, picker, panels);

    renderer_3D_update_uniforms(renderer, camera);

    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {renderer->renderState.vulkan.imageAvailableSemaphores[renderer->renderState.vulkan.currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame];
    VkSemaphore signalSemaphores[] = {renderer->renderState.vulkan.renderFinishedSemaphores[renderer->renderState.vulkan.currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    PFN_vkQueueSubmit pfnQueueSubmit =
        (PFN_vkQueueSubmit)glfwGetInstanceProcAddress(NULL, "vkQueueSubmit");
    PFN_vkQueuePresentKHR pfnQueuePresentKHR =
        (PFN_vkQueuePresentKHR)glfwGetInstanceProcAddress(NULL, "vkQueuePresentKHR");

    if (pfnQueueSubmit(renderer->renderState.vulkan.graphicsQueue, 1, &submitInfo, renderer->renderState.vulkan.inFlightFences[renderer->renderState.vulkan.currentFrame]) != VK_SUCCESS) {
        printf("failed to submit draw command buffer.\n");
        assert(false);
    }

    VkPresentInfoKHR presentInfo = { 0 };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {renderer->renderState.vulkan.swapChain.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    result = pfnQueuePresentKHR(renderer->renderState.vulkan.presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || renderer->renderState.vulkan.framebufferResized) {
        renderer->renderState.vulkan.framebufferResized = false;
        renderer_recreate_swap_chain(renderer);
    } else if (result != VK_SUCCESS) {
        printf("failed to present swap chain image.\n");
        assert(false);
    }

    renderer->renderState.vulkan.currentFrame = (renderer->renderState.vulkan.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void renderer_record_command_buffer(Renderer* renderer, VkCommandBuffer commandBuffer, uint32_t imageIndex, World* world, Camera* camera, Picker* picker, LinkedList* panels) {
    VkCommandBufferBeginInfo beginInfo = { 0 };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = NULL;

    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer =
        (PFN_vkBeginCommandBuffer)glfwGetInstanceProcAddress(NULL, "vkBeginCommandBuffer");

    if (pfnBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
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
    renderPassInfo.renderPass = renderer->renderState.vulkan.renderPass;
    renderPassInfo.framebuffer = renderer->renderState.vulkan.swapChain.frameBuffers[imageIndex];
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = renderer->renderState.vulkan.swapChain.extent;
    renderPassInfo.clearValueCount =(uint32_t)(sizeof(clearValues) / sizeof(clearValues[0]));
    renderPassInfo.pClearValues = clearValues;

    PFN_vkCmdBeginRenderPass pfnCmdBeginRenderPass =
        (PFN_vkCmdBeginRenderPass)glfwGetInstanceProcAddress(NULL, "vkCmdBeginRenderPass");

    pfnCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    PFN_vkCmdSetViewport pfnCmdSetViewport =
        (PFN_vkCmdSetViewport)glfwGetInstanceProcAddress(NULL, "vkCmdSetViewport");

    VkViewport viewport = { 0 };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)renderer->renderState.vulkan.swapChain.extent.width;
    viewport.height = (float)renderer->renderState.vulkan.swapChain.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    pfnCmdSetViewport(commandBuffer, 0, 1, &viewport);

    PFN_vkCmdSetScissor pfnCmdSetScissor =
        (PFN_vkCmdSetScissor)glfwGetInstanceProcAddress(NULL, "vkCmdSetScissor");

    VkRect2D scissor = { 0 };
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = renderer->renderState.vulkan.swapChain.extent;
    pfnCmdSetScissor(commandBuffer, 0, 1, &scissor);

    renderer_3D_record(renderer, world, camera, picker);

    PFN_vkCmdBindPipeline pfnCmdBindPipeline =
        (PFN_vkCmdBindPipeline)glfwGetInstanceProcAddress(NULL, "vkCmdBindPipeline");

    pfnCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->renderState.vulkan.pipeline2D.pipeline.pipeline);

    renderer_2D_record(renderer, panels);

    PFN_vkCmdEndRenderPass pfnCmdEndRenderPass =
        (PFN_vkCmdEndRenderPass)glfwGetInstanceProcAddress(NULL, "vkCmdEndRenderPass");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer =
        (PFN_vkEndCommandBuffer)glfwGetInstanceProcAddress(NULL, "vkEndCommandBuffer");

    pfnCmdEndRenderPass(commandBuffer);

    if (pfnEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        printf("failed to record command buffer.\n");
        assert(false);
    }
}

void renderer_3D_record_mesh(Renderer* renderer, Mesh* mesh, char mode) {
    PFN_vkCmdBindVertexBuffers pfnCmdBindVertexBuffers =
        (PFN_vkCmdBindVertexBuffers)glfwGetInstanceProcAddress(NULL, "vkCmdBindVertexBuffers");
    PFN_vkCmdBindIndexBuffer pfnCmdBindIndexBuffer =
        (PFN_vkCmdBindIndexBuffer)glfwGetInstanceProcAddress(NULL, "vkCmdBindIndexBuffer");
    PFN_vkCmdDrawIndexed pfnCmdDrawIndexed =
        (PFN_vkCmdDrawIndexed)glfwGetInstanceProcAddress(NULL, "vkCmdDrawIndexed");

    VkBuffer vertexBuffers[] = { mesh->renderState.vulkan.vbo };
    VkDeviceSize offsets[] = { 0 };
    pfnCmdBindVertexBuffers(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], 0, 1, vertexBuffers, offsets);
    pfnCmdBindIndexBuffer(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], mesh->renderState.vulkan.ebo, 0, VK_INDEX_TYPE_UINT16);
    int elementsPerQuad = (mode == MESH_FILL) ? 4 : 5;
    for (int i = 0; i < mesh->quads.size; i++) {
        pfnCmdDrawIndexed(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], elementsPerQuad, 1, elementsPerQuad * i, 0, 0);
    }
}

void renderer_3D_record_ground(Renderer* renderer, Ground* ground, Camera* camera) {
    PFN_vkCmdPushConstants pfnCmdPushConstants =
        (PFN_vkCmdPushConstants)glfwGetInstanceProcAddress(NULL, "vkCmdPushConstants");

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
    pfnCmdPushConstants(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], renderer->renderState.vulkan.pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 64, sizeof(worldPosition), worldPosition);
    // Color
    pfnCmdPushConstants(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], renderer->renderState.vulkan.pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 80, sizeof(color), color);

    renderer_3D_record_mesh(renderer, &ground->mesh, MESH_FILL);
}

void renderer_3D_record(Renderer* renderer, World* world, Camera* camera, Picker* picker) {
    PFN_vkCmdPushConstants pfnCmdPushConstants =
        (PFN_vkCmdPushConstants)glfwGetInstanceProcAddress(NULL, "vkCmdPushConstants");

    // Model matrix
    float mat4[16];
    mat4_identity(mat4);
    pfnCmdPushConstants(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], renderer->renderState.vulkan.pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(mat4), mat4);

    // Sun position
    float vec3[] = { 100, 100, 100 };
    pfnCmdPushConstants(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], renderer->renderState.vulkan.pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 96, sizeof(vec3), vec3);

    // Ambient
    float ambient = 0.2;
    pfnCmdPushConstants(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], renderer->renderState.vulkan.pipeline3D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 112, sizeof(ambient), &ambient);

    PFN_vkCmdBindPipeline pfnCmdBindPipeline =
        (PFN_vkCmdBindPipeline)glfwGetInstanceProcAddress(NULL, "vkCmdBindPipeline");
    PFN_vkCmdBindDescriptorSets pfnCmdBindDescriptorSets =
        (PFN_vkCmdBindDescriptorSets)glfwGetInstanceProcAddress(NULL, "vkCmdBindDescriptorSets");

    pfnCmdBindPipeline(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->renderState.vulkan.pipeline3D.pipeline.pipeline);
    pfnCmdBindDescriptorSets(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->renderState.vulkan.pipeline3D.pipeline.layout, 0, 1, &renderer->renderState.vulkan.pipeline3D.descriptorSets[renderer->renderState.vulkan.currentFrame], 0, NULL);

    renderer_3D_record_ground(renderer, &world->ground, camera);

    linked_list_foreach(&world->chunks, record_world_chunk, renderer);

    renderer_3D_record_picker(renderer, picker);
}

void renderer_2D_record(Renderer* renderer, LinkedList* panels) {
    PFN_vkCmdPushConstants pfnCmdPushConstants =
        (PFN_vkCmdPushConstants)glfwGetInstanceProcAddress(NULL, "vkCmdPushConstants");

    // Projection
    int width = 0, height = 0;
    glfwGetFramebufferSize(renderer->window->glfwWindow, &width, &height);

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(primary, &xscale, &yscale);

    float clipCorrect[16];
    vulkan_clip_correction_matrix(clipCorrect);

    float mat[16];

    mat4_orthographic(mat, 0, width / xscale, 0, height / yscale);
    mat4_multiply(mat, clipCorrect, mat);
    pfnCmdPushConstants(renderer->renderState.vulkan.commandBuffers[renderer->renderState.vulkan.currentFrame], renderer->renderState.vulkan.pipeline2D.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(mat), mat);

    linked_list_foreach(panels, record_panel, renderer);
}

void renderer_create_descriptor_sets(Renderer* renderer, VkImageView imageView, VkDescriptorSet** descriptorSets) {
    vulkan_create_descriptor_sets(renderer->vulkan->device, renderer->renderState.vulkan.descriptorPool, MAX_FRAMES_IN_FLIGHT, renderer->renderState.vulkan.pipeline2D.pipeline.descriptorSetLayout, descriptorSets);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorImageInfo imageInfo = { 0 };
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imageView;
        imageInfo.sampler = renderer->renderState.vulkan.pipeline2D.sampler;

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

        PFN_vkUpdateDescriptorSets pfnUpdateDescriptorSets =
            (PFN_vkUpdateDescriptorSets)glfwGetInstanceProcAddress(NULL, "vkUpdateDescriptorSets");

        pfnUpdateDescriptorSets(renderer->vulkan->device, 1, &descriptorWrite, 0, NULL);
    }
}
