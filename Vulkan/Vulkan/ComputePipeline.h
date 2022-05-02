#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include<vector>

class Device;

class ComputePipeline
{
private:
	Device& m_device;
	VkDescriptorSetLayout m_dSetLayout;
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipeline;

	VkShaderModule createShaderModule(std::vector<char> shader);

public:
	ComputePipeline(Device& device, const char* computePath);

	~ComputePipeline();
};

