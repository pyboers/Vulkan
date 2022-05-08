#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include<vector>
#include "vulkan/vulkan.hpp"

class Device;

class ComputePipeline
{
private:
	static vk::ShaderModule createShaderModule(const vk::Device &device, const std::vector<char> &shader);

	static vk::PipelineLayout createPipelineLayout(const vk::Device& device, const vk::DescriptorSetLayout &layouts);

	static vk::Pipeline createPipeline(const vk::Device& device, const vk::PipelineLayout& pipelineLayout, const char* computePath);
	
	Device& m_device;

public:
	vk::DescriptorSetLayout m_dSetLayout;
	vk::PipelineLayout m_pipelineLayout;
	vk::Pipeline m_pipeline;
	ComputePipeline(Device& device, const char* computePath, const vk::DescriptorSetLayout &descriptorSetLayout);

	~ComputePipeline();
};

