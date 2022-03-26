#ifndef PIPELINE_H_
#define PIPELINE_H_
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

struct Pipeline {
  static std::vector<char> readFile(const std::filesystem::path &path) {
    std::cout << "Path is: " << path << std::endl;
    /* ate: Read from the end of the file */
    /* binary: Read file as binary file */
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      throw std::runtime_error("[VkPipeline]: Reading shader file failed.");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
  }

  static void create(const VkDevice &device) {
    auto vertexShader = readFile(std::filesystem::path("../shaders/vert.spv"));
    auto fragShader = readFile(std::filesystem::path("../shaders/frag.spv"));

    VkShaderModule vertShaderModule = createShaderModule(vertexShader, device);
    VkShaderModule fragShaderModule = createShaderModule(fragShader, device);

    /* Can be made into a function. */
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

    /* For some reason shader modules are immediately deleted at the end of
     * pipeline creation */
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
  }

  static VkShaderModule createShaderModule(const std::vector<char> &code,
                                           const VkDevice &device) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) !=
        VK_SUCCESS) {
      throw std::runtime_error("[VkPipeline]: Since I can't create shader "
                               "module, no graphics for you.");
    }
    return shaderModule;
  }

  static void clean(const VkDevice &device) {}
};

#endif // PIPELINE_H_
