#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "swapchain.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>

struct Commands {
  static void createPool(const VkPhysicalDevice &physicalDevice,
                         const VkSurfaceKHR &surface, const VkDevice &device,
                         VkCommandPool &commandPool) {
    QueueFamilyIndices queueFamilyIndices =
        QueueFamilyIndices::find(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) !=
        VK_SUCCESS) {
      throw std::runtime_error("[VkCommands]: Uh... I don't have a pool.!");
    }
  }

  static void createBuffers(const VkDevice &device,
                            const VkCommandPool &commandPool,
                            VkCommandBuffer &commandBuffer) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) !=
        VK_SUCCESS) {
      throw std::runtime_error("[VkCommands]: I have pools, but I need "
                               "buffers. Please fix the buffers.");
    }
  }

  static void clean(const VkDevice &device, const VkCommandPool &commandPool) {
    vkDestroyCommandPool(device, commandPool, nullptr);
  }

  static void record(const VkCommandBuffer &commandBuffer, uint32_t imageIndex,
                     const VkRenderPass &renderPass,
                     const VkBuffer &vertexBuffer,
                     const std::vector<VkFramebuffer> &swapChainFramebuffers,
                     const VkExtent2D &swapChainExtent,
                     const VkPipeline &graphicsPipeline,
                     uint32_t size
    ) {

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                  // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("[VkCommands]: Recorder is stuck. Fix it.!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = {{{0.2f, 0.2f, 0.2f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphicsPipeline);

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdDraw(commandBuffer, size, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      throw std::runtime_error(
          "[VkCommands]: I was recording the something cut it off. Please "
          "re-record after you fix recording.!");
    }
  }
};

#endif // COMMANDS_H_
