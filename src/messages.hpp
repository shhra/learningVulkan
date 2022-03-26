#ifndef MESSAGES_H_
#define MESSAGES_H_

#include "settings.hpp"
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

struct Messages {
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData) {

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
      // Handle verbose message.
    } else if (messageSeverity ==
               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
      // Display infomration.
    } else if (messageSeverity >=
               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      // Only show critical message.
      std::cerr << "[VkMessage]: Validation says...  " << pCallbackData->pMessage
                << std::endl;
    }
    return VK_FALSE;
  }

  static void populate(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo = {};

    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    // skip verbose
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    // skip performance
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // It is optional
  }

  static VkResult
  createDebugMsgExt(VkInstance instance,
                    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                    const VkAllocationCallbacks *pAllocator,
                    VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
      return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
  }

  static void destroyDebugMsgExt(VkInstance instance,
                                 VkDebugUtilsMessengerEXT debugMessenger,
                                 const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
      func(instance, debugMessenger, pAllocator);
    }
  }
};

#endif // MESSAGES_H_
