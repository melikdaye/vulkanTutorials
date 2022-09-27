//
// Created by Administrator on 18.08.2022.
//

#ifndef VULKANTUTORIALS_VULKANRENDERER_H
#define VULKANTUTORIALS_VULKANRENDERER_H


#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "Utilities.h"
#include <set>
#include <stdexcept>
#include <algorithm>
#include <array>

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
        VkQueue presentationQueue;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapChain;
        std::vector<SwapChainImage> swapChainImages;

        VkPipeline graphicsPipeline;
        VkPipelineLayout pipelineLayout;
        VkRenderPass  renderPass;

        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;


        //Vulkan Functions
        // - Create Functions
        void createInstance();
        void createLogicalDevice();
        void createSurface();
        void createSwapChain();
        void createRenderPass();
        void createGraphicsPipeline();

        //- Get Functions

        void getPhysicalDevice();


        // Support Functions
        bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
        bool checkDeviceSuitable(VkPhysicalDevice device);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        // -- Getter Functions

        QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
        SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);

        // -- Choose FUnctions

        VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&formats);
        VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR>&presentationModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);

        VkImageView createImageView(VkImage image,VkFormat format,VkImageAspectFlags aspectFlags);
        VkShaderModule createShaderModule(const std::vector<char> &code);



};


#endif //VULKANTUTORIALS_VULKANRENDERER_H
