cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
project(graphics)

add_subdirectory(library/glfw)

include_directories(include)

add_executable(graphics
    src/main.cpp
    src/base.hpp
    src/device.hpp
    src/settings.hpp
    src/messages.hpp
    src/swapchain.hpp
    src/renderpass.hpp
    src/pipeline.hpp
)

target_link_libraries(graphics glfw ${GLFW_LIBRARIES} vulkan ${VULKAN_LIBRARIES})
