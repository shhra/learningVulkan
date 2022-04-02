#ifndef BUFFERS_H
#define BUFFERS_H
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

#endif /* BUFFERS_H */
