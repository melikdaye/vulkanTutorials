//
// Created by Administrator on 29.09.2022.
//

#ifndef VULKANTUTORIALS_MESH_H
#define VULKANTUTORIALS_MESH_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Utilities.h"

class Mesh {
    public:
        Mesh();

        Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, std::vector<Vertex> * vertices);

        int getVertexCount() {
            return vertexCount;
        }

        VkBuffer getVertexBuffer() {
            return vertexBuffer;
        }

        void destroyVertexBuffer(){
            vkDestroyBuffer(device,vertexBuffer, nullptr);
            vkFreeMemory(device,vertexBufferMemory, nullptr);
        }

    virtual ~Mesh();

    private:

        int vertexCount;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;

        VkPhysicalDevice physicalDevice;
        VkDevice device;

        void createVertexBuffer(std::vector<Vertex> *vertices);
        uint32_t findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags propertyFlags);
};


#endif //VULKANTUTORIALS_MESH_H
