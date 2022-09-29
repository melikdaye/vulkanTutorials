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
        void draw();
        void cleanup();

        ~VulkanRenderer();

    private:

        int currentFrame = 0;
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
        std::vector<VkFramebuffer> swapChainFrameBuffers;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkFence> drawFences;

        VkPipeline graphicsPipeline;
        VkPipelineLayout pipelineLayout;
        VkRenderPass  renderPass;

        VkCommandPool graphicsCommandPool;

        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;


        std::vector<VkSemaphore> imageAvailable;
        std::vector<VkSemaphore> renderFinished;




        //Vulkan Functions
        // - Create Functions
        void createInstance();
        void createLogicalDevice();
        void createSurface();
        void createSwapChain();
        void createRenderPass();
        void createGraphicsPipeline();
        void createFrameBuffers();
        void createCommandPool();
        void createCommandBuffers();
        void createSynchronisation();


        void recordCommands();

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
