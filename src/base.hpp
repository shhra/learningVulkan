#ifndef BASE_H_
#define BASE_H_

#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Report and handle the error
#include <iostream>
#include <stdexcept>

// Get the EXIT_SUCCESS and EXIT_FAILURE macros
#include "messages.hpp"
#include "settings.hpp"
#include <cstdlib>

// This is a typical vulkan application where vulkan objects are stored.
// The code is straight forward that provides function to load the bare
// vulkan objects.
namespace App {

class VkApp {

public:
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
  void initContext() {
    createInstance();
    setupDebugMessenger();
  }

  // Update the graphical elements.
  void loop() {
    // Make sure the window runs throughout the program
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
    }
  }

  // Free the allocated resources
  void clean() {
    if (enableValidationLayers) {
      // Messages::destroyDebugMsgExt(instance, debugMessenger, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  // Handle the vulkan instance creation
  void createInstance() {
    // If no validation exists, throw error
    if (enableValidationLayers && !Validation::checkLayerSupport()) {
      throw std::runtime_error(
          "Oops, I don't have validation support. Find it and give it to me.");
    }
    // Data about application - Optional
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "explorer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // Create instance - Required
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    VkDebugUtilsMessengerCreateInfoEXT dbgCreateInfo{};
    auto extensions = Extensions::get();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // Since we have the validation layer enabled we will have  to add the
    // information.
    if (enableValidationLayers) {
      createInfo.enabledLayerCount =
          static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();

      Messages::populate(dbgCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &dbgCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
      throw std::runtime_error("[VkApp]: Hey, I can't create new instance!");
    }
  }

  void setupDebugMessenger() {
    if (!enableValidationLayers)
      return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    Messages::populate(createInfo);

    if (Messages::createDebugMsgExt(instance, &createInfo, nullptr,
                                    &debugMessenger) != VK_SUCCESS) {
      throw std::runtime_error(
          "[VkApp]: Well, you will not get debugger. Sad....");
    }
  }

  // Window context
  GLFWwindow *window;
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
};

} // namespace App
#endif // BASE_H_
