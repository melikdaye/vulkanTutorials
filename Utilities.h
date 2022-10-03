//
// Created by Administrator on 22.08.2022.
//

#ifndef VULKANTUTORIALS_UTILITIES_H
#define VULKANTUTORIALS_UTILITIES_H

#include <vector>
#include <fstream>
#include "glm/glm.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const int MAX_FRAME_DRAWS = 2;

const std::vector<const char *>deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 col;

};

struct QueueFamilyIndices {
    int graphicsFamily = -1;
    int presentationFamily = -1;
    bool isValid(){
        return  graphicsFamily >= 0 && presentationFamily >=0;
    }
};

struct SwapChainDetails {
    VkSurfaceCapabilitiesKHR surfaceCapabilities; //Surface properties, q.g image size / extent
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentationModes;
};

struct SwapChainImage {
    VkImage image;
    VkImageView imageView;
};

static std::vector<char> readFile(const std::string &filename){

    std::ifstream file(filename,std::ios::binary | std::ios::ate);

    if(!file.is_open()){
        throw std::runtime_error("Failed to open a file!");
    }

    size_t fileSize = (size_t)file.tellg();

    std::vector<char> fileBuffer(fileSize);

    file.seekg(0);

    file.read(fileBuffer.data(),fileSize);

    file.close();

    return fileBuffer;
}

static uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice,uint32_t allowedTypes, VkMemoryPropertyFlags propertyFlags) {

    VkPhysicalDeviceMemoryProperties  memoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice,&memoryProperties);

    for (int i = 0; i < memoryProperties.memoryTypeCount; ++i) {

        if((allowedTypes & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags)) {

            return i;

        }
    }
}


static void createBuffer(VkPhysicalDevice physicalDevice,VkDevice device,VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags,VkMemoryPropertyFlags bufferProperties, VkBuffer *buffer, VkDeviceMemory *bufferMemory){
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = bufferSize;
    bufferCreateInfo.usage = bufferUsageFlags;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult  result = vkCreateBuffer(device,&bufferCreateInfo, nullptr,buffer);

    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a vertex buffer !");
    }

    VkMemoryRequirements memoryRequirements = {};
    vkGetBufferMemoryRequirements(device,*buffer,&memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice,memoryRequirements.memoryTypeBits,bufferProperties);

    result = vkAllocateMemory(device,&memoryAllocateInfo, nullptr,bufferMemory);

    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a vertex buffer memory !");
    }

    vkBindBufferMemory(device,*buffer,*bufferMemory,0);

}

static void copyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize){

    VkCommandBuffer transferCommandBuffer;

    VkCommandBufferAllocateInfo allocateInfo = {};

    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = transferCommandPool;
    allocateInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(device,&allocateInfo,&transferCommandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(transferCommandBuffer,&beginInfo);

    VkBufferCopy bufferCopyRegion = {};

    bufferCopyRegion.srcOffset = 0;
    bufferCopyRegion.dstOffset = 0;
    bufferCopyRegion.size = bufferSize;


    vkCmdCopyBuffer(transferCommandBuffer,srcBuffer,dstBuffer,1,&bufferCopyRegion);

    vkEndCommandBuffer(transferCommandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &transferCommandBuffer;

    vkQueueSubmit(transferQueue,1,&submitInfo,VK_NULL_HANDLE);

    vkQueueWaitIdle(transferQueue);

    vkFreeCommandBuffers(device,transferCommandPool,1,&transferCommandBuffer);
}

#endif //VULKANTUTORIALS_UTILITIES_H
