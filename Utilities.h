//
// Created by Administrator on 22.08.2022.
//

#ifndef VULKANTUTORIALS_UTILITIES_H
#define VULKANTUTORIALS_UTILITIES_H

#include <vector>
#include <fstream>


const std::vector<const char *>deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
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

#endif //VULKANTUTORIALS_UTILITIES_H
