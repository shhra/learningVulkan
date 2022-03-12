cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
project(graphics)

add_subdirectory(library/glfw)

include_directories(include)

add_executable(graphics
    src/main.cpp
    src/base.hpp
)

target_link_libraries(graphics glfw ${GLFW_LIBRARIES} vulkan ${VULKAN_LIBRARIES})
