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
  static VkBufferCreateInfo create(const VkDevice &device,
                                   VkBuffer &vertexBuffer, uint32_t buffer_size,
                                   VkBufferUsageFlags usage) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = buffer_size;
    bufferInfo.usage = usage;
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
                     VkBuffer &vertexBuffer, VkDeviceMemory &vertexBufferMemory,
                     const std::vector<Vertex> &vertices, void *data,
                     const VkCommandPool &commandPool,
                     const VkQueue &graphicsQueue) {

    // Vertex buffer is here.
    auto buffer_size = sizeof(vertices[0]) * vertices.size();

    // Staging buffer.
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    auto stagingBufferInfo = Buffers::create(device, stagingBuffer, buffer_size,
                                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    Allocation::allocate(device, physicalDevice, stagingBuffer,
                         stagingBufferMemory,
                         (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

    vkMapMemory(device, stagingBufferMemory, 0, buffer_size, 0, &data);
    mempcpy(data, vertices.data(), (size_t)buffer_size);
    vkUnmapMemory(device, stagingBufferMemory);


    auto bufferInfo = Buffers::create(device, vertexBuffer, buffer_size,
                                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    Allocation::allocate(device, physicalDevice, vertexBuffer,
                         vertexBufferMemory,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copy(stagingBuffer, vertexBuffer, buffer_size, commandPool, device, graphicsQueue);

    // Then destory the staging buffers.
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
  }

  static void copy(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size,
            const VkCommandPool &commandPool, const VkDevice &device,
            const VkQueue &graphicsQueue) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
  }
};

struct IndexBuffers {
  static void create(const VkDevice &device,
                     const VkPhysicalDevice &physicalDevice,
                     VkBuffer &indexBuffer, VkDeviceMemory &indexBufferMemory,
                     const std::vector<std::uint16_t> &indices, void *data,
                     const VkCommandPool &commandPool,
                     const VkQueue &graphicsQueue) {

    VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    auto stagingBufferInfo = Buffers::create(device, stagingBuffer, buffer_size,
                                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    Allocation::allocate(device, physicalDevice, stagingBuffer,
                         stagingBufferMemory,
                         (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

    vkMapMemory(device, stagingBufferMemory, 0, buffer_size, 0, &data);
    mempcpy(data, indices.data(), (size_t)buffer_size);
    vkUnmapMemory(device, stagingBufferMemory);


    auto bufferInfo = Buffers::create(device, indexBuffer, buffer_size,
                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    Allocation::allocate(device, physicalDevice, indexBuffer,
                         indexBufferMemory,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VertexBuffers::copy(stagingBuffer, indexBuffer, buffer_size, commandPool, device, graphicsQueue);

    // Then destory the staging buffers.
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

  }
};

#endif /* BUFFERS_H */
