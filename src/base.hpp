#ifndef BASE_H_
#define BASE_H_

#include <cstddef>
#include <cstdint>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Report and handle the error
#include <iostream>
#include <stdexcept>

// Get the EXIT_SUCCESS and EXIT_FAILURE macros
#include <cstdlib>

// This is a typical vulkan application where vulkan objects are stored.
// The code is straight forward that provides function to load the bare
// vulkan objects.
namespace App {

class VkApp {

public:
  static const uint32_t WIDTH = 800;
  static const uint32_t HEIGHT = 600;
  void run() {
    initWindow();
    initContext();
    loop();
    clean();
  }

private:
  // This adds glfw window.
  void initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "explorer", nullptr, nullptr);
  }
  // Allocate resources
  void initContext() {}

  // Update the graphical elements.
  void loop() {
    // Make sure the window runs throughout the program
    while(!glfwWindowShouldClose(window)) {
      glfwPollEvents();
    }
  }

  // Free the allocated resources
  void clean() {
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  // Window context
  GLFWwindow *window;
};

} // namespace App
#endif // BASE_H_
