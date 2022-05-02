#include "ComputePipeline.h"
#include "Device.h"
#include <stdexcept>
#include "Utils.h"

VkShaderModule ComputePipeline::createShaderModule(std::vector<char> shader)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shader.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device.m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

ComputePipeline::ComputePipeline(Device& device, const char* computePath)
:m_device(device) {
    std::vector<char> compute = readFile(computePath);

    VkShaderModule computeModule = createShaderModule(compute);

    VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[2] = {
      {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
      {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0}
    };

    VkDescriptorSetLayoutCreateInfo dSetLayoutInfo{};
    dSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dSetLayoutInfo.flags = 0;
    dSetLayoutInfo.pNext = nullptr;
    dSetLayoutInfo.bindingCount = 2;
    dSetLayoutInfo.pBindings = descriptorSetLayoutBindings;

    if (vkCreateDescriptorSetLayout(m_device.m_device, &dSetLayoutInfo, nullptr, &m_dSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.flags = 0;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &m_dSetLayout;
    layoutInfo.pushConstantRangeCount = 0;
    layoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(m_device.m_device, &layoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.flags = 0;
    shaderStageInfo.pNext = nullptr;
    shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageInfo.module = computeModule;
    shaderStageInfo.pName = "main";
    shaderStageInfo.pSpecializationInfo = nullptr;

    VkComputePipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.stage = shaderStageInfo;
    createInfo.layout = m_pipelineLayout;
    createInfo.basePipelineIndex = 0;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateComputePipelines(m_device.m_device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline");
    }

    vkDestroyShaderModule(m_device.m_device, computeModule, nullptr);
}

ComputePipeline::~ComputePipeline()
{
    vkDestroyDescriptorSetLayout(m_device.m_device, m_dSetLayout, nullptr);
    vkDestroyPipelineLayout(m_device.m_device, m_pipelineLayout, nullptr);
    vkDestroyPipeline(m_device.m_device, m_pipeline, nullptr);
}
