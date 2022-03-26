#ifndef DEVICE_H_
#define DEVICE_H_
#include "swapchain.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

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

  void instantiateLogical(VkDeviceCreateInfo &info, VkDevice &logical) {
    if (vkCreateDevice(this->physicalDevice, &info, nullptr, &logical) !=
        VK_SUCCESS) {
      throw std::runtime_error("[VkDevice]: Logical device...... denied.");
    }
    std::cout << "[VkDevice]: You now have a logical device." << std::endl;
  }

  uint32_t extsSize() { return static_cast<uint32_t>(deviceExtensions.size()); }

  const std::vector<const char *> &exts() { return deviceExtensions; }

  const VkPhysicalDevice &get() { return physicalDevice; }

private:
  /* Add extension */
  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  bool isSuitable(const VkPhysicalDevice &device, const VkSurfaceKHR &surface) {
    QueueFamilyIndices indices = QueueFamilyIndices::find(device, surface);
    bool extensionSupported = checkExtSupport(device);
    bool swapChainAdequate = false;
    if (extensionSupported) {
      SwapChain support;
      support = SwapChain::query(device, surface);
      /* Make sure this device has some supported format and presentation mode
       */
      swapChainAdequate =
          !support.formats.empty() && !support.presentModes.empty();
    }

    return indices.isComplete() && extensionSupported && swapChainAdequate;
  }

  bool checkExtSupport(const VkPhysicalDevice &device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                             deviceExtensions.end());

    for (const auto &extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
  }
};

class LogicalDevice {
public:
  void createLogicalDevice(PhysicalDevice &physicalDevice,
                           const VkSurfaceKHR &surface) {
    QueueFamilyIndices indices =
        QueueFamilyIndices::find(physicalDevice.get(), surface);

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
    createInfo.enabledExtensionCount = physicalDevice.extsSize();
    createInfo.ppEnabledExtensionNames = physicalDevice.exts().data();

    /* Instantiate the logical deivce. */
    physicalDevice.instantiateLogical(createInfo, device);

    /* Get the queue */
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &presentQueue);
  }

  VkQueue &queue() { return this->presentQueue; }

  void clean() { vkDestroyDevice(device, nullptr); }

  const VkDevice &get() { return device; }

private:
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
};

#endif // DEVICE_H_
