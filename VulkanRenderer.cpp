//
// Created by Administrator on 18.08.2022.
//

#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer() {

}

VulkanRenderer::~VulkanRenderer() {

}

int VulkanRenderer::init(GLFWwindow *newWindow) {

    window = newWindow;
    try {
        createInstance();
        createSurface();
        getPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createRenderPass();
        createGraphicsPipeline();
        createFrameBuffers();
        createCommandPool();

        std::vector<Vertex> meshVertices = {
                {{-0.1,-0.4,0.0},{1.0f,0.0f,0.0f}},
                {{-0.1,0.4,0.0},{0.0f,1.0f,0.0f}},
                {{-0.9,0.4,0.0},{0.0f,0.0f,1.0f}},
                {{-0.9,-0.4,0.0},{1.0f,0.0f,1.0f}},
        };

        std::vector<Vertex> meshVertices2 = {
                {{0.9,-0.4,0.0},{1.0f,0.0f,0.0f}},
                {{0.9,0.4,0.0},{0.0f,1.0f,0.0f}},
                {{0.1,0.4,0.0},{0.0f,0.0f,1.0f}},
                {{0.1,-0.4,0.0},{1.0f,0.0f,1.0f}},
        };

        std::vector<uint32_t> meshIndices = {
                0,1,2,2,3,0
        };

        Mesh firstMesh = Mesh(mainDevice.physicalDevice,mainDevice.logicalDevice,graphicsQueue,graphicsCommandPool,&meshVertices,&meshIndices);
        Mesh secondMesh = Mesh(mainDevice.physicalDevice,mainDevice.logicalDevice,graphicsQueue,graphicsCommandPool,&meshVertices2,&meshIndices);

        meshList.push_back(firstMesh);
        meshList.push_back(secondMesh);

        createCommandBuffers();
        recordCommands();
        createSynchronisation();
    }
    catch (const std::runtime_error &e){
        printf("ERROR %s \n",e.what());
        return EXIT_FAILURE;
    }

    return 0;

}

void VulkanRenderer::draw() {

    vkWaitForFences(mainDevice.logicalDevice,1,&drawFences[currentFrame],VK_TRUE,std::numeric_limits<uint64_t>::max());
    vkResetFences(mainDevice.logicalDevice,1,&drawFences[currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(mainDevice.logicalDevice,swapChain,std::numeric_limits<uint64_t>::max(),imageAvailable[currentFrame],VK_NULL_HANDLE,&imageIndex);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailable[currentFrame];
    VkPipelineStageFlags  waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinished[currentFrame];

    VkResult  result = vkQueueSubmit(graphicsQueue,1,&submitInfo,drawFences[currentFrame]);

    if(VK_SUCCESS != result){
        throw  std::runtime_error("Failed to submit command buffer to queue!");
    }

    VkPresentInfoKHR presentInfoKhr = {};
    presentInfoKhr.sType = VK_STRUCTURE_TYPE_PRESENT_ID_KHR;
    presentInfoKhr.waitSemaphoreCount = 1;
    presentInfoKhr.pWaitSemaphores = &renderFinished[currentFrame];
    presentInfoKhr.swapchainCount = 1;
    presentInfoKhr.pSwapchains = &swapChain;
    presentInfoKhr.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentationQueue,&presentInfoKhr);
    if(VK_SUCCESS != result){
        throw  std::runtime_error("Failed to present image!");
    }

    currentFrame = (currentFrame+1) % MAX_FRAME_DRAWS;


}


void VulkanRenderer::createInstance() {

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char *> instanceExtensions = std::vector<const char*>();
    uint32_t glfwExtensionCount = 0;
    const char ** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (int i = 0; i < glfwExtensionCount; ++i) {
        instanceExtensions.push_back(glfwExtensions[i]);
    }
    if(!checkInstanceExtensionSupport(&instanceExtensions)){
        throw std::runtime_error("VkInstance does not support required extensions!");
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;

    VkResult  result = vkCreateInstance(&createInfo, nullptr,&instance);
    if(VK_SUCCESS != result){
        throw  std::runtime_error("Failed to create Vulkan Instance");
    }else{
        printf("Vulkan instance is created successfully");
    }
}

void VulkanRenderer::createSurface() {

    VkResult result = glfwCreateWindowSurface(instance,window, nullptr,&surface);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a surface!");
    }
}

void VulkanRenderer::getPhysicalDevice() {

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance,&deviceCount, nullptr);

    if(deviceCount == 0)
        throw std::runtime_error("Cannot find GPUs that support Vulkan Instance!");

    std::vector<VkPhysicalDevice> deviceList(deviceCount);
    vkEnumeratePhysicalDevices(instance,&deviceCount, deviceList.data());

    for (const auto  &device: deviceList) {
        if(checkDeviceSuitable(device)){
            mainDevice.physicalDevice = device;
            break;
        }
    }

}

void VulkanRenderer::createLogicalDevice() {

    QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> queueFamilyIndices = {indices.graphicsFamily, indices.presentationFamily};

    for(int queueFamilyIndex : queueFamilyIndices) {

        VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        deviceQueueCreateInfo.queueCount = 1;
        float priority = 1.f;
        deviceQueueCreateInfo.pQueuePriorities = &priority;

        queueCreateInfos.push_back(deviceQueueCreateInfo);
    }
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VkResult result = vkCreateDevice(mainDevice.physicalDevice,&deviceCreateInfo, nullptr,&mainDevice.logicalDevice);

    if(VK_SUCCESS != result) {
        throw std::runtime_error("Failed to create a logical device !");
    }

    vkGetDeviceQueue(mainDevice.logicalDevice,indices.graphicsFamily,0,&graphicsQueue);
    vkGetDeviceQueue(mainDevice.logicalDevice,indices.presentationFamily,0,&presentationQueue);

}

void VulkanRenderer::createSwapChain() {
    SwapChainDetails swapChainDetails  = getSwapChainDetails(mainDevice.physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseBestSurfaceFormat(swapChainDetails.formats);
    VkPresentModeKHR presentMode = chooseBestPresentationMode(swapChainDetails.presentationModes);
    VkExtent2D extent = chooseSwapExtent(swapChainDetails.surfaceCapabilities);

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = surface;
    swapChainCreateInfo.imageFormat = surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.imageExtent = extent;
    swapChainCreateInfo.minImageCount = std::max(static_cast<uint32_t>(1),std::min(swapChainDetails.surfaceCapabilities.maxImageCount,swapChainDetails.surfaceCapabilities.minImageCount + 1));
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.clipped = VK_TRUE;

    QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);

    if(indices.graphicsFamily != indices.presentationFamily){

        uint32_t queueFamilyIndices[] = {
                (int32_t) indices.graphicsFamily, (int32_t) indices.presentationFamily
        };
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }else{
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(mainDevice.logicalDevice,&swapChainCreateInfo, nullptr,&swapChain);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create swapchain !");
    }

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(mainDevice.logicalDevice,swapChain,&swapChainImageCount, nullptr);
    std::vector<VkImage> images(swapChainImageCount);
    vkGetSwapchainImagesKHR(mainDevice.logicalDevice,swapChain,&swapChainImageCount, images.data());

    for (VkImage image:images) {
        SwapChainImage swapChainImage = {};
        swapChainImage.image = image;

        swapChainImage.imageView = createImageView(image,swapChainImageFormat,VK_IMAGE_ASPECT_COLOR_BIT);

        swapChainImages.push_back(swapChainImage);

    }

}

void VulkanRenderer::createRenderPass() {

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;

    std::array<VkSubpassDependency,2> subpassDependencies;

    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    subpassDependencies[0].dstSubpass = 0;
    subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dependencyFlags = 0;

    subpassDependencies[1].srcSubpass = 0;
    subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].dstStageMask =  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[1].dependencyFlags = 0;


    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
    renderPassCreateInfo.pDependencies = subpassDependencies.data();

    VkResult  result = vkCreateRenderPass(mainDevice.logicalDevice,&renderPassCreateInfo, nullptr,&renderPass);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a render pass !");
    }

}

void VulkanRenderer::createGraphicsPipeline() {

    auto vertexShaderCode = readFile("../Shaders/vert.spv");
    auto fragmentShaderCode = readFile("../Shaders/frag.spv");

    VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
    VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);


    VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
    vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderCreateInfo.module = vertexShaderModule;
    vertexShaderCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
    fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderCreateInfo.module = fragmentShaderModule;
    fragmentShaderCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderCreateInfo,fragmentShaderCreateInfo};


    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription,2> attributeDescriptions;

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex,pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex,col);

    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;


    VkViewport viewport = {};
    viewport.x  = 0.0f;
    viewport.y  = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0,0};
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportCreateInfo = {};
    viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportCreateInfo.pViewports = &viewport;
    viewportCreateInfo.viewportCount = 1;
    viewportCreateInfo.pScissors = &scissor;
    viewportCreateInfo.scissorCount = 1;

    std::vector<VkDynamicState> dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();


    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
    rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.depthClampEnable = VK_FALSE;
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerCreateInfo.lineWidth = 1.0f;
    rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizerCreateInfo.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
    multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |  VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentState.blendEnable = VK_TRUE;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo blendCreateInfo = {};
    blendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendCreateInfo.logicOpEnable = VK_FALSE;
    blendCreateInfo.attachmentCount = 1;
    blendCreateInfo.pAttachments = &colorBlendAttachmentState;

    VkPipelineLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.setLayoutCount = 0;
    layoutCreateInfo.pSetLayouts = nullptr;
    layoutCreateInfo.pushConstantRangeCount = 0;
    layoutCreateInfo.pPushConstantRanges = nullptr;

    VkResult result = vkCreatePipelineLayout(mainDevice.logicalDevice,&layoutCreateInfo, nullptr,&pipelineLayout);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a pipeline layout !");
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
    pipelineCreateInfo.pViewportState = &viewportCreateInfo;
    pipelineCreateInfo.pDynamicState = nullptr;
    pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
    pipelineCreateInfo.pColorBlendState = &blendCreateInfo;
    pipelineCreateInfo.pDepthStencilState = nullptr;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    result = vkCreateGraphicsPipelines(mainDevice.logicalDevice,VK_NULL_HANDLE,1,&pipelineCreateInfo, nullptr,&graphicsPipeline);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a graphics pipeline !");
    }



    vkDestroyShaderModule(mainDevice.logicalDevice,fragmentShaderModule, nullptr);
    vkDestroyShaderModule(mainDevice.logicalDevice,vertexShaderModule, nullptr);

}


void VulkanRenderer::createFrameBuffers() {
    swapChainFrameBuffers.resize(swapChainImages.size());

    for(size_t i = 0; i< swapChainFrameBuffers.size(); i++){

        std::array<VkImageView,1> attachments = {swapChainImages[i].imageView};

        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = renderPass;
        framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferCreateInfo.pAttachments = attachments.data();
        framebufferCreateInfo.width = swapChainExtent.width;
        framebufferCreateInfo.height = swapChainExtent.height;
        framebufferCreateInfo.layers = 1;

        VkResult  result = vkCreateFramebuffer(mainDevice.logicalDevice,&framebufferCreateInfo, nullptr,&swapChainFrameBuffers[i]);

        if(result != VK_SUCCESS){
            throw std::runtime_error("Failed to create a framebuffer !");
        }
    }
}

void VulkanRenderer::createCommandPool() {

    QueueFamilyIndices queueFamilyIndices = getQueueFamilies(mainDevice.physicalDevice);

    VkCommandPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    VkResult result = vkCreateCommandPool(mainDevice.logicalDevice,&poolCreateInfo, nullptr,&graphicsCommandPool);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a command pool !");
    }
}

void VulkanRenderer::createCommandBuffers() {

    commandBuffers.resize(swapChainFrameBuffers.size());

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = graphicsCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    VkResult result = vkAllocateCommandBuffers(mainDevice.logicalDevice,&commandBufferAllocateInfo,commandBuffers.data());
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate a command buffers !");
    }

}

void VulkanRenderer::recordCommands() {

    VkCommandBufferBeginInfo bufferBeginInfo = {};
    bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//    bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset = {0,0};
    renderPassBeginInfo.renderArea.extent = swapChainExtent;
    VkClearValue clearValues[] = {
            {0.6f,0.65f,0.4f,1.0f}
    };
    renderPassBeginInfo.pClearValues = clearValues;
    renderPassBeginInfo.clearValueCount = 1;

    for (size_t i = 0; i < commandBuffers.size(); ++i) {

        renderPassBeginInfo.framebuffer = swapChainFrameBuffers[i];

        VkResult result = vkBeginCommandBuffer(commandBuffers[i],&bufferBeginInfo);

        if(result != VK_SUCCESS){
            throw std::runtime_error("Failed to start recording a command buffer !");
        }


        vkCmdBeginRenderPass(commandBuffers[i],&renderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[i],VK_PIPELINE_BIND_POINT_GRAPHICS,graphicsPipeline);

        for (int j = 0; j < meshList.size(); ++j) {
            VkBuffer vertexBuffers[] = {meshList[j].getVertexBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[i],0,1,vertexBuffers,offsets);
            vkCmdBindIndexBuffer(commandBuffers[i],meshList[j].getIndexBuffer(),0,VK_INDEX_TYPE_UINT32);

//        vkCmdDraw(commandBuffers[i],static_cast<uint32_t>(firstMesh.getVertexCount()),1,0,0);
            vkCmdDrawIndexed(commandBuffers[i],static_cast<uint32_t>(meshList[j].getIndexCount()),1,0,0,0);
        }


        vkCmdEndRenderPass(commandBuffers[i]);

        result = vkEndCommandBuffer(commandBuffers[i]);

        if(result != VK_SUCCESS){
            throw std::runtime_error("Failed to stop recording a command buffer !");
        }
    }

}

void VulkanRenderer::createSynchronisation() {

    imageAvailable.resize(MAX_FRAME_DRAWS);
    renderFinished.resize(MAX_FRAME_DRAWS);
    drawFences.resize(MAX_FRAME_DRAWS);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAME_DRAWS; ++i) {
        if(vkCreateSemaphore(mainDevice.logicalDevice,&semaphoreCreateInfo, nullptr,&imageAvailable[i]) != VK_SUCCESS ||
           vkCreateSemaphore(mainDevice.logicalDevice,&semaphoreCreateInfo, nullptr,&renderFinished[i]) != VK_SUCCESS ||
                vkCreateFence(mainDevice.logicalDevice,&fenceCreateInfo, nullptr,&drawFences[i])){
            throw std::runtime_error("Failed to stop create a semaphore !");
        }
    }

}

void VulkanRenderer::cleanup() {

    vkDeviceWaitIdle(mainDevice.logicalDevice);

    for (int i = 0; i < meshList.size() ; ++i) {
        meshList[i].destroyBuffers();
    }

    for (size_t i = 0; i < MAX_FRAME_DRAWS; ++i) {
        vkDestroySemaphore(mainDevice.logicalDevice, renderFinished[i], nullptr);
        vkDestroySemaphore(mainDevice.logicalDevice, imageAvailable[i], nullptr);
        vkDestroyFence(mainDevice.logicalDevice,drawFences[i], nullptr);
    }
    vkDestroyCommandPool(mainDevice.logicalDevice,graphicsCommandPool, nullptr);
    for (auto framebuffer: swapChainFrameBuffers) {
        vkDestroyFramebuffer(mainDevice.logicalDevice,framebuffer, nullptr);
    }
    vkDestroyPipeline(mainDevice.logicalDevice,graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(mainDevice.logicalDevice,pipelineLayout, nullptr);
    vkDestroyRenderPass(mainDevice.logicalDevice,renderPass, nullptr);
    for (auto image:swapChainImages) {
        vkDestroyImageView(mainDevice.logicalDevice,image.imageView, nullptr);
    }
    vkDestroySwapchainKHR(mainDevice.logicalDevice,swapChain, nullptr);
    vkDestroySurfaceKHR(instance,surface, nullptr);
    vkDestroyDevice(mainDevice.logicalDevice, nullptr);
    vkDestroyInstance(instance, nullptr);
}

bool VulkanRenderer::checkInstanceExtensionSupport(std::vector<const char *> *checkExtensions) {

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr,&extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr,&extensionCount,extensions.data());
    for (const auto &checkExtension: *checkExtensions) {
        bool hasExtension = false;
        for (const auto &extension: extensions) {
            if (strcmp(checkExtension,extension.extensionName)){
                hasExtension = true;
                break;
            }
        }
        if(!hasExtension){
            return false;
        }
    }
    return true;
}


bool VulkanRenderer::checkDeviceSuitable(VkPhysicalDevice device) {

//    VkPhysicalDeviceProperties deviceProperties;
//    vkGetPhysicalDeviceProperties(device,&deviceProperties);
//
//    VkPhysicalDeviceFeatures deviceFeatures;
//    vkGetPhysicalDeviceFeatures(device,&deviceFeatures);

    QueueFamilyIndices indices = getQueueFamilies(device);

    bool extensionSupported = checkDeviceExtensionSupport(device);

    bool swapChainValid = false;

    if(extensionSupported) {
        SwapChainDetails swapChainDetails = getSwapChainDetails(device);
        swapChainValid = !swapChainDetails.presentationModes.empty() && !swapChainDetails.formats.empty();
    }

    return indices.isValid() && extensionSupported && swapChainValid;
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t  queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device,&queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device,&queueFamilyCount,queueFamilyList.data());
    int i = 0;
    for (const auto &queueFamily: queueFamilyList) {
        if(queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            indices.graphicsFamily = i;
        }
        VkBool32 presentationSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device,i,surface,&presentationSupport);
        if(queueFamily.queueCount >0 && presentationSupport){
            indices.presentationFamily = i;
        }
        if(indices.isValid()){
            break;
        }
        i++;
    }
    return  indices;
}





bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t  extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr,&extensionCount, nullptr);
    if(extensionCount == 0){
        return false;
    }
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr,&extensionCount,extensions.data());
    bool hasExtension = false;
    for(const auto &deviceExtension:deviceExtensions){
        for (const auto &extension:extensions) {
            if(strcmp(deviceExtension,extension.extensionName) == 0){
                hasExtension = true;
                break;
            }
        }
    }
    return hasExtension;
}

SwapChainDetails VulkanRenderer::getSwapChainDetails(VkPhysicalDevice device) {

    SwapChainDetails swapChainDetails;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device,surface,&swapChainDetails.surfaceCapabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device,surface,&formatCount, nullptr);
    if(formatCount!=0){
        swapChainDetails.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device,surface,&formatCount,swapChainDetails.formats.data());
    }

    uint32_t presentationCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device,surface,&presentationCount, nullptr);
    if(presentationCount != 0){
        swapChainDetails.presentationModes.resize(presentationCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device,surface,&presentationCount, swapChainDetails.presentationModes.data());
    }

    return swapChainDetails;
}




VkSurfaceFormatKHR VulkanRenderer::chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {

    if(formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED){
        return {VK_FORMAT_R8G8B8A8_UNORM,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }
    for (const auto &format : formats){
        if((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            return format;
        }
    }
    return formats[0];
}

VkPresentModeKHR VulkanRenderer::chooseBestPresentationMode(const std::vector<VkPresentModeKHR> &presentationModes) {
    for (const auto &presentationMode: presentationModes) {
        if(presentationMode == VK_PRESENT_MODE_MAILBOX_KHR){
            return presentationMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
    if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){
        return surfaceCapabilities.currentExtent;
    }
    else{
        int width,height;
        glfwGetFramebufferSize(window,&width,&height);

        VkExtent2D newExtent = {};
        newExtent.width = static_cast<uint32_t>(width);
        newExtent.height = static_cast<uint32_t>(height);

        newExtent.width = std::max(surfaceCapabilities.minImageExtent.width,std::min(surfaceCapabilities.maxImageExtent.width,newExtent.width));
        newExtent.height = std::max(surfaceCapabilities.minImageExtent.height,std::min(surfaceCapabilities.maxImageExtent.height,newExtent.height));

        return newExtent;

    }
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {

    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = image;
    viewCreateInfo.format = format;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VkResult  result = vkCreateImageView(mainDevice.logicalDevice,&viewCreateInfo, nullptr,&imageView);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create image view !");
    }

    return imageView;
}



VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char> &code) {
    VkShaderModuleCreateInfo shaderModuleCreateInfo;
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    VkResult  result = vkCreateShaderModule(mainDevice.logicalDevice,&shaderModuleCreateInfo, nullptr,&shaderModule);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a shader module !");
    }

    return shaderModule;

}







