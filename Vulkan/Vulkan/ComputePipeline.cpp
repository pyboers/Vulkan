#include "ComputePipeline.h"
#include "Device.h"
#include <stdexcept>
#include "Utils.h"

vk::ShaderModule ComputePipeline::createShaderModule(const vk::Device& device, const std::vector<char>& shader)
{
    const size_t size = shader.size();
    const uint32_t* c = reinterpret_cast<const uint32_t*>(shader.data());
    vk::ShaderModuleCreateInfo createInfo(vk::ShaderModuleCreateFlags(), size, c);

    return device.createShaderModule(createInfo);
}

vk::PipelineLayout ComputePipeline::createPipelineLayout(const vk::Device& device, const vk::DescriptorSetLayout &layouts)
{
    vk::PipelineLayoutCreateInfo createInfo({}, layouts);

    return device.createPipelineLayout(createInfo);
}

vk::Pipeline ComputePipeline::createPipeline(const vk::Device& device, const vk::PipelineLayout& pipelineLayout, const char* computePath)
{
    vk::ShaderModule shaderModule = createShaderModule(device, readFile(computePath));
    vk::PipelineShaderStageCreateInfo shaderCreateInfo({}, vk::ShaderStageFlagBits::eCompute, shaderModule, "main");
    vk::ComputePipelineCreateInfo createInfo({}, shaderCreateInfo, pipelineLayout);

    vk::ResultValue<vk::Pipeline> result = device.createComputePipeline(nullptr, createInfo);
    vkDestroyShaderModule(device, shaderModule, nullptr);
    vk::resultCheck(result.result, "Pipeline creation failed!");

    return result.value;
}

ComputePipeline::ComputePipeline(Device& device, const char* computePath, const vk::DescriptorSetLayout& descriptorSetLayout)
:m_device(device),
m_dSetLayout(descriptorSetLayout),
m_pipelineLayout(createPipelineLayout(device.m_device, descriptorSetLayout)),
m_pipeline(createPipeline(device.m_device, m_pipelineLayout, computePath))
{
  
}

ComputePipeline::~ComputePipeline()
{
    vkDestroyDescriptorSetLayout(m_device.m_device, m_dSetLayout, nullptr);
    vkDestroyPipelineLayout(m_device.m_device, m_pipelineLayout, nullptr);
    vkDestroyPipeline(m_device.m_device, m_pipeline, nullptr);
}
