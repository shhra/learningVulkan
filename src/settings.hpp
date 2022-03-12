#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <GLFW/glfw3.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

static const uint32_t WIDTH = 800;
static const uint32_t HEIGHT = 600;
static const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

// Only enable validations for debug builds
#ifdef NDEBUG
static const bool enableValidationLayers = false;
#else
static const bool enableValidationLayers = true;
#endif

struct Validation {
  // This just checks if the our validation layers are present or not.
  static bool checkLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers) {
      bool found = false;
      for (const auto &layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          found = true;
          break;
        }
      }

      if (!found) {
        return false;
      }
    }
    return true;
  }
};

struct Extensions {
  // Bundles the always required GLFW extensions along with the validations
  static std::vector<const char *> get() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    // Why do we add the final pointer?
    // TODO: Verify the meaning of this.
    std::vector<const char *> extensions(glfwExtensions,
                                         glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
  }
};

#endif // SETTINGS_H_
