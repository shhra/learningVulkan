#include <cstdlib>
#include <exception>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "base.hpp"

#define GLFW_FORCE_RADIANS
#include <iostream>

int main() {

  VkApp app;
  try {
    app.run();
  } catch (std::exception& e) {
    // As soon as we except any fatal error, print it out.
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
