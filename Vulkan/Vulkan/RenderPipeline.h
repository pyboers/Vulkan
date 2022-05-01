#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include<vector>

class Device;

class RenderPipeline
{
private:
	Device& m_device;
	VkRenderPass m_renderPass;
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipeline;

	VkShaderModule createShaderModule(std::vector<char> shader);

	VkRenderPass createRenderPass(VkFormat colorFormat);

public:
	RenderPipeline(Device &device, const char* vertPath, const char* fragPath, VkExtent2D extents, VkFormat colorFormat);

	~RenderPipeline();
};

