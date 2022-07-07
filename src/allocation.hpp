#ifndef ALLOCATION_H_
#define ALLOCATION_H_
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

struct Allocation {

  static uint32_t findMemoryType(const VkPhysicalDevice &physicalDevice,
                                 uint32_t &typeFilter,
                                 VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) &&
          // part is interesting because we are simply comparing if there are
          // two same flags
          (memProperties.memoryTypes[i].propertyFlags & properties) ==
              properties) {
        return i;
      }
    }

    throw std::runtime_error(
        "[VkMemory]: failed to find suitable memory type!");
  }

  static void allocate(const VkDevice &device,
                       const VkPhysicalDevice &physicalDevice, VkBuffer &buffer,
                       VkDeviceMemory &bufferMemory,
                       VkMemoryPropertyFlags properties) {
    // Memory Related
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};

    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) !=
        VK_SUCCESS) {
      throw std::runtime_error(
          "[VkMemory]: Oh boy, I can't allocate my memory!");
    }
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
  }

  static void free(const VkDevice &device, VkDeviceMemory &bufferMemory) {
    vkFreeMemory(device, bufferMemory, nullptr);
  }
};

#endif // ALLOCATION_H_
