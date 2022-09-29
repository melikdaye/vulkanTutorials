cmake_minimum_required(VERSION 3.19.2)
project(vulkanTutorials)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

include_directories(D:/GLFW/include)
include_directories(D:/VulkanSDK/1.3.216.0/Include)

link_directories(D:/GLFW/lib-vc2012)
link_directories(D:/VulkanSDK/1.3.216.0/Lib32)

set(CMAKE_CXX_STANDARD 11)



add_executable(vulkanTutorials  main.cpp VulkanRenderer.cpp VulkanRenderer.h Mesh.cpp Mesh.h)

target_link_libraries(vulkanTutorials vulkan-1 glfw3)