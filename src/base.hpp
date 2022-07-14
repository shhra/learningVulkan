#ifndef BASE_H_
#define BASE_H_

#include "allocation.hpp"
#include "buffers.hpp"
#include "commands.hpp"
#include "pipeline.hpp"
#include "renderpass.hpp"
#include "swapchain.hpp"
#include "vertex.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Report and handle the error
#include <iostream>
#include <stdexcept>

// Get the EXIT_SUCCESS and EXIT_FAILURE macros
#include "device.hpp"
#include "messages.hpp"
#include "settings.hpp"
#include <cstdlib>

// This is a typical vulkan application where vulkan objects are stored.
// The code is straight forward that provides function to load the bare
// vulkan objects.
namespace App {

class VkApp {

public:
  void run() {
    initWindow();
    initContext();
    loop();
    clean();
  }

private:
  // This adds glfw window.
  void initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "explorer", nullptr, nullptr);
  }
  // Allocate resources
  void initContext() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    physicalDevice.pick(instance, surface);
    device.createLogicalDevice(physicalDevice, surface);
    SwapChain::create(window, physicalDevice.get(), surface, device.get(),
                      &swapChain, swapChainImages, swapChainImageFormat,
                      swapChainExtent);
    createImageViews();
    RenderPass::create(device.get(), swapChainImageFormat, renderPass);
    Pipeline::create(device.get(), swapChainExtent, pipelineLayout, renderPass,
                     graphicsPipeline);
    FrameBuffers::create(device.get(), renderPass, swapChainFramebuffers,
                         swapChainImageViews, swapChainExtent);
    Commands::createPool(physicalDevice.get(), surface, device.get(),
                         commandPool);
    VertexBuffers::create(device.get(), physicalDevice.get(), vertexBuffer,
                          vertexBufferMemory, vertices, data, commandPool,
                          device.gQueue());
    IndexBuffers::create(device.get(), physicalDevice.get(), indexBuffer,
                          indexBufferMemory, indices, index_data, commandPool,
                          device.gQueue());
    Commands::createBuffers(device.get(), commandPool, commandBuffer);
    createSyncObjects();
  }

  // Update the graphical elements.
  void loop() {
    // Make sure the window runs throughout the program
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      drawFrame();
    }
    vkDeviceWaitIdle(device.get());
  }

  void drawFrame() {
    auto device = this->device.get();
    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
                          imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(commandBuffer, 0);
    auto size = static_cast<uint32_t>(vertices.size());

    Commands::record(commandBuffer, imageIndex, renderPass, vertexBuffer, indexBuffer,
                     swapChainFramebuffers, swapChainExtent, graphicsPipeline,
                     size, static_cast<uint32_t>(indices.size()));

    /* Submit info */
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    /* Specify command buffers */
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(this->device.gQueue(), 1, &submitInfo, inFlightFence) !=
        VK_SUCCESS) {
      throw std::runtime_error(
          "[VkApp]: You half baked commands doesn't make me tick. Fix it.");
    }

    /* Drawing */
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(this->device.queue(), &presentInfo);
  }

  // Free the allocated resources
  void clean() {
    vkDestroySemaphore(device.get(), imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(device.get(), renderFinishedSemaphore, nullptr);
    vkDestroyFence(device.get(), inFlightFence, nullptr);

    Commands::clean(device.get(), commandPool);
    FrameBuffers::clean(device.get(), swapChainFramebuffers);
    Pipeline::clean(device.get(), pipelineLayout, graphicsPipeline);
    RenderPass::clean(device.get(), renderPass);
    for (auto imageView : swapChainImageViews) {
      vkDestroyImageView(device.get(), imageView, nullptr);
    }
    vkDestroySwapchainKHR(device.get(), swapChain, nullptr);
    Buffers::clean(device.get(), indexBuffer);
    Allocation::free(device.get(), indexBufferMemory);

    Buffers::clean(device.get(), vertexBuffer);
    Allocation::free(device.get(), vertexBufferMemory);

    if (enableValidationLayers) {
      Messages::destroyDebugMsgExt(instance, debugMessenger, nullptr);
    }
    device.clean();
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  // Handle the vulkan instance creation
  void createInstance() {
    // If no validation exists, throw error
    if (enableValidationLayers && !Validation::checkLayerSupport()) {
      throw std::runtime_error(
          "Oops, I don't have validation support. Find it and give it to me.");
    }
    // Data about application - Optional
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "explorer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // Create instance - Required
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    VkDebugUtilsMessengerCreateInfoEXT dbgCreateInfo{};
    auto extensions = Extensions::get();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // Since we have the validation layer enabled we will have  to add the
    // information.
    if (enableValidationLayers) {
      createInfo.enabledLayerCount =
          static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();

      Messages::populate(dbgCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&dbgCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
      throw std::runtime_error("[VkApp]: Hey, I can't create new instance!");
    }
  }

  void createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) !=
        VK_SUCCESS) {
      throw std::runtime_error("[VkApp]: Surface denied...., must need one.!");
    }
  }

  void createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
      VkImageViewCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = swapChainImages[i];

      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format = swapChainImageFormat;

      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;
      if (vkCreateImageView(device.get(), &createInfo, nullptr,
                            &swapChainImageViews[i]) != VK_SUCCESS) {
        throw std::runtime_error(
            "[VkApp]: You don't have views, you can't see!");
      }
    }
  }

  void setupDebugMessenger() {
    if (!enableValidationLayers)
      return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    Messages::populate(createInfo);

    if (Messages::createDebugMsgExt(instance, &createInfo, nullptr,
                                    &debugMessenger) != VK_SUCCESS) {
      throw std::runtime_error(
          "[VkApp]: Well, you will not get debugger. Sad....");
    }
  }

  void createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateSemaphore(device.get(), &semaphoreInfo, nullptr,
                          &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device.get(), &semaphoreInfo, nullptr,
                          &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device.get(), &fenceInfo, nullptr, &inFlightFence) !=
            VK_SUCCESS) {
      throw std::runtime_error("[VkApp]: I will mess your pixels. There is no "
                               "way I want to synchronize. It slows me.!");
    }
  }

  // Window context
  GLFWwindow *window;
  VkInstance instance;
  VkSurfaceKHR surface;
  VkDebugUtilsMessengerEXT debugMessenger;
  PhysicalDevice physicalDevice;
  LogicalDevice device;
  VkSwapchainKHR swapChain;
  VkRenderPass renderPass;
  VkPipeline graphicsPipeline;
  VkCommandPool commandPool;
  VkCommandBuffer commandBuffer;
  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;
  VkFence inFlightFence;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;

  // Load object
  const std::vector<Vertex> vertices = Shape::create();
  const std::vector<std::uint16_t> indices = Shape::indices();

  // Map
  void *data;
  void *index_data;

  // Swap chain related.
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  VkPipelineLayout pipelineLayout;
};

} // namespace App
#endif // BASE_H_
