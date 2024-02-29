#include "vulkan_util.h"

#ifdef NODEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

const char* deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const VkDynamicState dynamicStates[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

int vulkan_validation_layers_count() {
    return sizeof(validationLayers) / sizeof(validationLayers[0]);
}

const char** vulkan_validation_layers() {
    return validationLayers;
}

bool vulkan_check_validation_layer_support() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties* availableLayers = NEW(VkLayerProperties, layerCount);

    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    int validationLayersCount = sizeof(validationLayers) / sizeof(validationLayers[0]);
        bool layerFound = false;
    for (int i = 0; i < validationLayersCount; i++) {
        layerFound = false;

        for (int j = 0; j < layerCount; j++) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            break;
        }
    }

    free(availableLayers);

    return layerFound;
}

bool vulkan_queue_family_indices_is_complete(QueueFamilyIndices* indices) {
    return indices->graphicsFamily != (uint32_t)-1 && indices->presentFamily != (uint32_t)-1;
}

QueueFamilyIndices vulkan_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;
    indices.graphicsFamily = -1;
    indices.presentFamily = -1;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties* queueFamilies = NEW(VkQueueFamilyProperties, queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for (int i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (vulkan_queue_family_indices_is_complete(&indices)) {
            break;
        }
    }

    free(queueFamilies);

    return indices;
}

SwapChainSupportDetails vulkan_query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details = { 0 };

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);

    if (formatCount != 0) {
        details.formats = NEW(VkSurfaceFormatKHR, formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);

    if (presentModeCount != 0) {
        details.presentModes = NEW(VkPresentModeKHR, presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes);
    }

    details.numFormats = formatCount;
    details.numPresentModes = presentModeCount;

    return details;
}

bool vulkan_check_device_extension_support(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    VkExtensionProperties* availableExtensions = NEW(VkExtensionProperties, extensionCount);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    int deviceExtensionCount = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);
    for (int i = 0; i < deviceExtensionCount; i++) {
        bool found = false;
        for (int j = 0; j < extensionCount; j++) {
            if (strcmp(deviceExtensions[i], availableExtensions[j].extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            free(availableExtensions);
            return false;
        }
    }

    free(availableExtensions);
    return true;
}

bool vulkan_is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = vulkan_find_queue_families(device, surface);

    bool extensionsSupported = vulkan_check_device_extension_support(device);

    bool swapChainAdequate = false;

    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = vulkan_query_swap_chain_support(device, surface);
        swapChainAdequate = swapChainSupport.numFormats > 0 && swapChainSupport.numPresentModes > 0;
        free(swapChainSupport.formats);
        free(swapChainSupport.presentModes);
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return vulkan_queue_family_indices_is_complete(&indices) && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

void vulkan_create_instance(const char* appName, VkInstance* instance) {
    if (enableValidationLayers && !vulkan_check_validation_layer_support()) {
        printf("validation layers requested, but not available.\n");
        assert(false);
    }

    VkApplicationInfo appInfo = { 0 };
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = vulkan_validation_layers_count();
        createInfo.ppEnabledLayerNames = vulkan_validation_layers();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if(vkCreateInstance(&createInfo, NULL, instance) != VK_SUCCESS) {
        printf("failed to create Vulkan instance.\n");
        assert(false);
    }
}

void vulkan_pick_physical_device(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* physicalDevice) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        printf("failed to find GPUs with Vulkan support.\n");
        assert(false);
    }

    *physicalDevice = VK_NULL_HANDLE;

    VkPhysicalDevice* devices = NEW(VkPhysicalDevice, deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    for (int i = 0; i < deviceCount; i++) {
        if (vulkan_is_device_suitable(devices[i], surface)) {
            *physicalDevice = devices[i];
            break;
        }
    }

    if (*physicalDevice == VK_NULL_HANDLE) {
        printf("failed to find a suitable GPU.\n");
        while(true);
    }

    free(devices);
}

void vulkan_create_logical_device(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice* device, VkQueue* graphicsQueue, VkQueue* presentQueue) {
    QueueFamilyIndices indices = vulkan_find_queue_families(physicalDevice, surface);

    VkDeviceQueueCreateInfo* queueCreateInfos;
    int queueCreateInfosSize;

    float queuePriority = 1.0f;
    if (indices.graphicsFamily == indices.presentFamily) {
        queueCreateInfosSize = 1;
        queueCreateInfos = NEW(VkDeviceQueueCreateInfo, 1);

        VkDeviceQueueCreateInfo info = { 0 };
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = indices.graphicsFamily;
        info.queueCount = 1;
        info.pQueuePriorities = &queuePriority;
        queueCreateInfos[0] = info;

    } else {
        queueCreateInfosSize = 2;
        queueCreateInfos = NEW(VkDeviceQueueCreateInfo, 2);

        VkDeviceQueueCreateInfo info1 = { 0 };
        info1.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info1.queueFamilyIndex = indices.graphicsFamily;
        info1.queueCount = 1;
        info1.pQueuePriorities = &queuePriority;
        queueCreateInfos[0] = info1;

        VkDeviceQueueCreateInfo info2 = { 0 };
        info2.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info2.queueFamilyIndex = indices.presentFamily;
        info2.queueCount = 1;
        info2.pQueuePriorities = &queuePriority;
        queueCreateInfos[1] = info2;
    }

    VkPhysicalDeviceFeatures devicesFeatures = { 0 };
    devicesFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfosSize;
    createInfo.pEnabledFeatures = &devicesFeatures;

    createInfo.enabledExtensionCount = (uint32_t)(sizeof(deviceExtensions) / sizeof(deviceExtensions[0]));
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = (uint32_t)(sizeof(validationLayers) / sizeof(validationLayers[0]));
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, NULL, device) != VK_SUCCESS) {
        printf("failed to create logical device.\n");
        assert(false);
    }

    free(queueCreateInfos);

    vkGetDeviceQueue(*device, indices.graphicsFamily, 0, graphicsQueue);
    vkGetDeviceQueue(*device, indices.presentFamily, 0, presentQueue);
}

VkSurfaceFormatKHR vulkan_choose_swap_surface_format(SwapChainSupportDetails* supportDetails) {
    for (int i = 0; i < supportDetails->numFormats; i++) {
        if (supportDetails->formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && supportDetails->formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return supportDetails->formats[i];
        }
    }

    return supportDetails->formats[0];
}

VkPresentModeKHR vulkan_choose_swap_present_mode(SwapChainSupportDetails* supportDetails) {
    for (int i = 0; i < supportDetails->numPresentModes; i++) {
        if (supportDetails->presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return supportDetails->presentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D vulkan_choose_swap_extent(SwapChainSupportDetails* supportDetails, GLFWwindow* glfwWindow) {
    if (supportDetails->capabilities.currentExtent.width != UINT32_MAX) {
        return supportDetails->capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(glfwWindow, &width, &height);

        VkExtent2D actualExtent = {
            (uint32_t)width,
            (uint32_t)height
        };

        if (actualExtent.width < supportDetails->capabilities.minImageExtent.width) {
            actualExtent.width = supportDetails->capabilities.minImageExtent.width;
        }
        if (actualExtent.width > supportDetails->capabilities.maxImageExtent.width) {
            actualExtent.width = supportDetails->capabilities.maxImageExtent.width;
        }

        if (actualExtent.height < supportDetails->capabilities.minImageExtent.height) {
            actualExtent.height = supportDetails->capabilities.minImageExtent.height;
        }
        if (actualExtent.height > supportDetails->capabilities.maxImageExtent.height) {
            actualExtent.height = supportDetails->capabilities.maxImageExtent.height;
        }

        return actualExtent;
    }
}

VkImageView vulkan_create_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo = { 0 };
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, NULL, &imageView) != VK_SUCCESS) {
        printf("failed to create image view.\n");
        assert(false);
    }

    return imageView;
}

void vulkan_create_swap_chain(VkPhysicalDevice physicalDevice, VkDevice device, GLFWwindow* glfwWindow, VkSurfaceKHR surface, SwapChain* swapChain) {
    SwapChainSupportDetails swapChainSupport = vulkan_query_swap_chain_support(physicalDevice, surface);

    VkSurfaceFormatKHR surfaceFormat = vulkan_choose_swap_surface_format(&swapChainSupport);
    VkPresentModeKHR presentMode = vulkan_choose_swap_present_mode(&swapChainSupport);
    VkExtent2D extent = vulkan_choose_swap_extent(&swapChainSupport, glfwWindow);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = vulkan_find_queue_families(physicalDevice, surface);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain->swapChain) != VK_SUCCESS) {
        printf("failed to create swap chain.\n");
        assert(false);
    }

    vkGetSwapchainImagesKHR(device, swapChain->swapChain, &imageCount, NULL);
    swapChain->images = NEW(VkImage, imageCount);
    swapChain->imageCount = imageCount;
    vkGetSwapchainImagesKHR(device, swapChain->swapChain, &imageCount, swapChain->images);

    swapChain->imageFormat = surfaceFormat.format;
    swapChain->extent = extent;

    swapChain->imageViews = NEW(VkImageView, imageCount);
    for (int i = 0; i < imageCount; i++) {
        swapChain->imageViews[i] = vulkan_create_image_view(device, swapChain->images[i], swapChain->imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    swapChain->frameBuffers = NEW(VkFramebuffer, imageCount);

    free(swapChainSupport.formats);
    free(swapChainSupport.presentModes);
}

VkFormat vulkan_find_supported_format(VkPhysicalDevice physicalDevice, VkFormat* candidates, int count, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (int i = 0; i < count; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i], &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return candidates[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return candidates[i];
        }
    }

    printf("failed to find supported format.\n");
    assert(false);

    return -1;
}

VkFormat vulkan_find_depth_format(VkPhysicalDevice physicalDevice) {
    VkFormat formats[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
    
    return vulkan_find_supported_format(
        physicalDevice,
        formats,
        sizeof(formats) / sizeof(formats[0]),
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void vulkan_create_render_pass(VkPhysicalDevice physicalDevice, VkDevice device, SwapChain* swapChain, VkRenderPass* renderPass) {
    VkAttachmentDescription colorAttachment = { 0 };
    colorAttachment.format = swapChain->imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = { 0 };
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = { 0 };
    depthAttachment.format = vulkan_find_depth_format(physicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = { 0 };
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = { 0 };
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo = { 0 };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = (uint32_t)(sizeof(attachments) / sizeof(attachments[0]));
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency = { 0 };
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, NULL, renderPass) != VK_SUCCESS) {
        printf("failed to create render pass.\n");
        assert(false);
    }
}

VkShaderModule vulkan_create_shader_module(VkDevice device, const char* name) {
    char fullpath[25];
    sprintf(fullpath, "shaders/%s", name);

    FILE* file;
    long fsize;
    char* buffer;

    file = fopen(fullpath, "rb");

    fseek(file, 0, SEEK_END);
    fsize = ftell(file);
    rewind(file);

    buffer = NEW(char, fsize);
    fread(buffer,1,fsize,file);

    fclose(file);

    VkShaderModuleCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = fsize;
    createInfo.pCode = (uint32_t*)buffer;

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        printf("failed to create shader module.\n");
        assert(false);
    }

    free(buffer);
    return shaderModule;
}

void vulkan_create_pipeline(VkDevice device, VkPipelineShaderStageCreateInfo* vertInfo, VkPipelineShaderStageCreateInfo* fragInfo, VkVertexInputBindingDescription* vertexBindingDescription, VkVertexInputAttributeDescription* vertexAttributeDescriptions, int vertexAttributeDescriptionCount, VkPushConstantRange* pushConstants, VkRenderPass renderPass, Pipeline* pipeline) {
    VkPipelineShaderStageCreateInfo shaderStages[] = {*vertInfo, *fragInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = { 0 };
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = vertexBindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptionCount;
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { 0 };
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineDynamicStateCreateInfo dynamicState = { 0 };
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = (uint32_t)(sizeof(dynamicStates) / sizeof(dynamicStates[0]));
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineViewportStateCreateInfo viewportState = { 0 };
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = { 0 };
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = { 0 };
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = NULL;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = { 0 };
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = { 0 };
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { 0 };
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &pipeline->descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = pushConstants;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipeline->layout) != VK_SUCCESS) {
        printf("failed to create pipeline layout.\n");
        assert(false);
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil = { 0 };
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkGraphicsPipelineCreateInfo pipelineInfo = { 0 };
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = pipeline->layout;

    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pipeline->pipeline) != VK_SUCCESS) {
        printf("failed to create graphics pipeline.\n");
        assert(false);
    }
}

void vulkan_create_command_pool(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkCommandPool* commandPool) {
    QueueFamilyIndices queueFamilyIndices = vulkan_find_queue_families(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo = { 0 };
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(device, &poolInfo, NULL, commandPool) != VK_SUCCESS) {
        printf("failed to create command pool.\n");
        assert(false);
    }
}

uint32_t vulkan_find_memory_type(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties ) {
            return i;
        }
    }

    printf("failed to find suitable memory type.\n");
    assert(false);

    return -1;
}

void vulkan_create_image(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory) {
    VkImageCreateInfo imageInfo = { 0 };
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(device, &imageInfo, NULL, image) != VK_SUCCESS) {
        printf("failed to create image.\n");
        assert(false);
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vulkan_find_memory_type(physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, NULL, imageMemory) != VK_SUCCESS) {
        printf("failed to allocate image memory.\n");
        assert(false);
    }

    vkBindImageMemory(device, *image, *imageMemory, 0);
}

void vulkan_create_sampler(VkPhysicalDevice physicalDevice, VkDevice device, VkSampler* sampler) {
    VkPhysicalDeviceProperties properties = { 0 };
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo = { 0 };
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(device, &samplerInfo, NULL, sampler) != VK_SUCCESS) {
        printf("failed to create texture sampler.\n");
        assert(false);
    }
}

void vulkan_create_buffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
    VkBufferCreateInfo bufferInfo = { 0 };
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        printf("failed to create vertex buffer.\n");
        assert(false);
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vulkan_find_memory_type(physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
        printf("failed to allocate vertex buffer memory.\n");
        assert(false);
    }

    vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

void vulkan_create_descriptor_set_pool(VkDevice device, VkDescriptorPoolSize* poolSizes, int numPoolSizes, VkDescriptorPool* descriptorPool, uint32_t maxSets) {
    VkDescriptorPoolCreateInfo poolInfo = { 0 };
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = (uint32_t)(numPoolSizes);
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = maxSets;

    if (vkCreateDescriptorPool(device, &poolInfo, NULL, descriptorPool) != VK_SUCCESS) {
        printf("failed to create descriptor pool.\n");
        assert(false);
    }
}

void vulkan_create_descriptor_sets(VkDevice device, VkDescriptorPool descriptorPool, int count, VkDescriptorSetLayout layout, VkDescriptorSet** descriptorSets) {
    VkDescriptorSetLayout* layouts = NEW(VkDescriptorSetLayout, count);
    for (int i = 0; i < count; i++) {
        layouts[i] = layout;
    }
    VkDescriptorSetAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = (uint32_t)count;
    allocInfo.pSetLayouts = layouts;

    *descriptorSets = NEW(VkDescriptorSet, count);
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, *descriptorSets);
    if (result != VK_SUCCESS) {
        printf("failed to allocate descriptor sets.\n");
        printf("%d\n", result);
        assert(false);
    }
}

void vulkan_create_command_buffers(VkDevice device, VkCommandPool commandPool, int count, VkCommandBuffer* commandBuffers) {
    VkCommandBufferAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = count;

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers) != VK_SUCCESS) {
        printf("failed to allocate command buffers.\n");
        assert(false);
    }
}

VkCommandBuffer vulkan_begin_single_time_commands(VkDevice device, VkCommandPool commandPool) {
    VkCommandBufferAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo= { 0 };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void vulkan_end_single_time_commands(VkCommandBuffer commandBuffer, VkDevice device, VkQueue queue, VkCommandPool commandPool) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void vulkan_copy_buffer(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = vulkan_begin_single_time_commands(device, commandPool);

    VkBufferCopy copyRegion = { 0 };
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vulkan_end_single_time_commands(commandBuffer, device, queue, commandPool);
}

void vulkan_transition_image_layout(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = vulkan_begin_single_time_commands(device, commandPool);

    VkImageMemoryBarrier barrier = { 0 };
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        printf("unsupported layout transition.\n");
        assert(false);
    }

    vkCmdPipelineBarrier(commandBuffer,
                            sourceStage, destinationStage,
                            0,
                            0, NULL,
                            0, NULL,
                            1, &barrier);

    vulkan_end_single_time_commands(commandBuffer, device, queue, commandPool);
}

void vulkan_copy_buffer_to_image(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = vulkan_begin_single_time_commands(device, commandPool);

    VkBufferImageCopy region = { 0 };
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    vulkan_end_single_time_commands(commandBuffer, device, queue, commandPool);
}