#ifndef DEVICE_H_
#define DEVICE_H_
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

/**
 * This setups the physical device. This is actual graphics card that is
 * available on the system. With vulkan we can score the quality of graphics
 * card and select one that suits our needs.
 * */
class PhysicalDevice {
public:
  void pick(const VkInstance &instance, const VkSurfaceKHR &surface) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
      throw std::runtime_error("[VkDevice]: I am sorry, I can't find the GPU.");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto &device : devices) {
      if (isSuitable(device, surface)) {
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

  QueueFamilyIndices findQueueFamilies(const VkSurfaceKHR &surface) {
    return findQueueFamilies(this->physicalDevice, surface);
  }

  void instantiateLogical(VkDeviceCreateInfo &info, VkDevice &logical) {
    if (vkCreateDevice(this->physicalDevice, &info, nullptr, &logical) !=
        VK_SUCCESS) {
      throw std::runtime_error("[VkDevice]: Logical device...... denied.");
    }
    std::cout << "[VkDevice]: You now have a logical device." << std::endl;
  }

private:
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  bool isSuitable(const VkPhysicalDevice &device, const VkSurfaceKHR &surface) {
    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    return indices.isComplete();
  }

  QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice &device,
                                       const VkSurfaceKHR &surface) {
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
      /* Check for the presentation support */
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

      if (presentSupport) {
        indices.presentFamily = i;
      }
      i++;
    }

    return indices;
  }
};

class LogicalDevice {
public:
  void createLogicalDevice(PhysicalDevice &physicalDevice,
                           const VkSurfaceKHR &surface) {
    QueueFamilyIndices indices = physicalDevice.findQueueFamilies(surface);

    /* Queue information. */
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                              indices.presentFamily.value()};
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    /* Device features. */
    VkPhysicalDeviceFeatures deviceFeatures{};

    /* Logical device */
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;

    /* Instantiate the logical deivce. */
    physicalDevice.instantiateLogical(createInfo, device);

    /* Get the queue */
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &presentQueue);
  }

  VkQueue &queue() { return this->presentQueue; }

  void clean() { vkDestroyDevice(device, nullptr); }

private:
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
};

#endif // DEVICE_H_
