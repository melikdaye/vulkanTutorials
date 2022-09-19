//
// Created by Administrator on 18.08.2022.
//

#include "VulkanRenderer.h"

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

int VulkanRenderer::init(GLFWwindow *newWindow) {

    window = newWindow;
    try {
        createInstance();
        getPhysicalDevice();
        createLogicalDevice();
    }catch (const std::runtime_error &e){
        printf("ERROR %s \n",e.what());
        return EXIT_FAILURE;
    }

    return 0;

}

VulkanRenderer::VulkanRenderer() {

}

VulkanRenderer::~VulkanRenderer() {

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

void VulkanRenderer::cleanup() {
    vkDestroyDevice(mainDevice.logicalDevice, nullptr);
    vkDestroyInstance(instance, nullptr);
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

bool VulkanRenderer::checkDeviceSuitable(VkPhysicalDevice device) {

//    VkPhysicalDeviceProperties deviceProperties;
//    vkGetPhysicalDeviceProperties(device,&deviceProperties);
//
//    VkPhysicalDeviceFeatures deviceFeatures;
//    vkGetPhysicalDeviceFeatures(device,&deviceFeatures);

    QueueFamilyIndices indices = getQueueFamilies(device);

    return indices.isValid();
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
        if(indices.isValid()){
            break;
        }
        i++;
    }
    return  indices;
}

void VulkanRenderer::createLogicalDevice() {

    QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);

    VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
    deviceQueueCreateInfo.queueCount = 1;
    float priority = 1.f;
    deviceQueueCreateInfo.pQueuePriorities = &priority;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = 0;
    deviceCreateInfo.ppEnabledExtensionNames = nullptr;

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VkResult result = vkCreateDevice(mainDevice.physicalDevice,&deviceCreateInfo, nullptr,&mainDevice.logicalDevice);

    if(VK_SUCCESS != result) {
        throw std::runtime_error("Failed to create a logical device !");
    }

    vkGetDeviceQueue(mainDevice.logicalDevice,indices.graphicsFamily,0,&graphicsQueue);

}
