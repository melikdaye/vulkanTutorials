//
// Created by Administrator on 18.08.2022.
//

#ifndef VULKANTUTORIALS_VULKANRENDERER_H
#define VULKANTUTORIALS_VULKANRENDERER_H

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "Utilities.h"

#include <stdexcept>
#include <vector>

class VulkanRenderer {
    public:
        VulkanRenderer();

        int init(GLFWwindow * newWindow);
        void cleanup();

        ~VulkanRenderer();

    private:
        GLFWwindow  *window;

        //Vulkan components;
        VkInstance  instance;
        struct{
            VkPhysicalDevice physicalDevice;
            VkDevice logicalDevice;
        }mainDevice;

        VkQueue graphicsQueue;

        //Vulkan Functions

        void createInstance();
        void createLogicalDevice();


        //- Get Functions

        void getPhysicalDevice();

        // Support Functions
        bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
        bool checkDeviceSuitable(VkPhysicalDevice device);

        // -- Getter Functions

        QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);


};


#endif //VULKANTUTORIALS_VULKANRENDERER_H
