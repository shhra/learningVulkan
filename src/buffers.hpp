#ifndef BUFFERS_H
#define BUFFERS_H
#include "allocation.hpp"
#include "vertex.hpp"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

struct FrameBuffers {
  static void create(const VkDevice &device, const VkRenderPass &renderPass,
                     std::vector<VkFramebuffer> &swapChainFramebuffers,
                     const std::vector<VkImageView> &swapChainImageViews,
                     const VkExtent2D &swapChainExtent) {
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
      VkImageView attachments[] = {swapChainImageViews[i]};

      VkFramebufferCreateInfo framebufferInfo{};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = attachments;
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      if (vkCreateFramebuffer(device, &framebufferInfo, nullptr,
                              &swapChainFramebuffers[i]) != VK_SUCCESS) {
        throw std::runtime_error("[VkFrameBuffer]: I want to display images. "
                                 "Please give me frame buffers.");
      }
    }
  }

  static void clean(const VkDevice &device,
                    const std::vector<VkFramebuffer> &swapChainFramebuffers) {
    for (auto framebuffer : swapChainFramebuffers) {
      vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
  }
};

struct Buffers {
  static VkBufferCreateInfo
  create(const VkDevice &device, VkBuffer &vertexBuffer, uint32_t buffer_size) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = buffer_size;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) !=
        VK_SUCCESS) {
      throw std::runtime_error("[VkVertexBuffer]: Lol, I don't have vertices. "
                               "You want me to display a blank screen?!");
    }

    return bufferInfo;
  }

  static void clean(const VkDevice &device, const VkBuffer &buffer) {
    vkDestroyBuffer(device, buffer, nullptr);
  }
};

struct VertexBuffers {
  static void create(const VkDevice &device,
                     const VkPhysicalDevice &physicalDevice,
                     VkBuffer &vertexBuffer, VkDeviceMemory vertexBufferMemory,
                     const std::vector<Vertex>& vertices) {


    auto buffer_size = sizeof(vertices[0]) * vertices.size();

    auto bufferInfo = Buffers::create(device, vertexBuffer, buffer_size);
    Allocation::allocate(device, physicalDevice, vertexBuffer,
                         vertexBufferMemory);

    // Map
    void *data;
    vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);
    mempcpy(data, vertices.data(), (size_t) bufferInfo.size);
    vkUnmapMemory(device, vertexBufferMemory);
  }
};

#endif /* BUFFERS_H */
