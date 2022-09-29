//
// Created by Administrator on 29.09.2022.
//

#include "Mesh.h"

Mesh::Mesh() {}

Mesh::~Mesh() {

}

Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, std::vector<Vertex> *vertices) {

    vertexCount = vertices->size();
    physicalDevice = newPhysicalDevice;
    device = newDevice;
    createVertexBuffer(vertices);
}


void Mesh::createVertexBuffer(std::vector<Vertex> *vertices) {

    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = sizeof(Vertex) * vertices->size();
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult  result = vkCreateBuffer(device,&bufferCreateInfo, nullptr,&vertexBuffer);

    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a vertex buffer !");
    }

    VkMemoryRequirements memoryRequirements = {};
    vkGetBufferMemoryRequirements(device,vertexBuffer,&memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(memoryRequirements.memoryTypeBits,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    result = vkAllocateMemory(device,&memoryAllocateInfo, nullptr,&vertexBufferMemory);

    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a vertex buffer memory !");
    }

    vkBindBufferMemory(device,vertexBuffer,vertexBufferMemory,0);
}

uint32_t Mesh::findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags propertyFlags) {

    VkPhysicalDeviceMemoryProperties  memoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice,&memoryProperties);

    for (int i = 0; i < memoryProperties.memoryTypeCount; ++i) {

        if((allowedTypes & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags)) {

            return i;

        }
    }
}


