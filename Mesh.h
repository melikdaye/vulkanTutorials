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

        Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex> * vertices,std::vector<uint32_t> *indices);

        int getVertexCount() {
            return vertexCount;
        }

        int getIndexCount() {
            return indexCount;
        }

        VkBuffer getVertexBuffer() {
            return vertexBuffer;
        }

        VkBuffer getIndexBuffer() {
            return indexBuffer;
        }

        void destroyBuffers(){
            vkDestroyBuffer(device,vertexBuffer, nullptr);
            vkFreeMemory(device,vertexBufferMemory, nullptr);
            vkDestroyBuffer(device,indexBuffer, nullptr);
            vkFreeMemory(device,indexBufferMemory, nullptr);
        }

    virtual ~Mesh();

    private:

        int vertexCount;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;

        int indexCount;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;


        VkPhysicalDevice physicalDevice;
        VkDevice device;

        void createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool,std::vector<Vertex> *vertices);
        void createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool,std::vector<uint32_t> *indices);

};


#endif //VULKANTUTORIALS_MESH_H
