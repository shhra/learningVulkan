#ifndef DEVICE_H_
#define DEVICE_H_
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;

  bool isComplete() { return graphicsFamily.has_value(); }
};

class Device {
public:
  void pickPhysical(const VkInstance &instance) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
      throw std::runtime_error("[VkDevice]: I am sorry, I can't find the GPU.");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto &device : devices) {
      if (isSuitable(device)) {
        physicalDevice = device;
        std::cout << "[VkDevice]: Hey, there is a gpu." << std::endl;
        break;
      }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
      throw std::runtime_error(
          "[VkDevice]: We can't proceed if you don't have a GPU.");
    }
  }

private:
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  bool isSuitable(const VkPhysicalDevice &device) {
    QueueFamilyIndices indices = findQueueFamilies(device);
    return indices.isComplete();
  }

  QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice &device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies) {
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphicsFamily = i;
      }
      i++;
    }

    return indices;
  }
};

#endif // DEVICE_H_
