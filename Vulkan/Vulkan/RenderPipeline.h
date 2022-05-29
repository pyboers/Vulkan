#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include<vector>
#include "vulkan/vulkan.hpp"
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

class Device;

class RenderPipeline
{
private:
	static vk::ShaderModule createShaderModule(const vk::Device& device, const std::vector<char>& shader);
	static vk::RenderPass createRenderPass(const vk::Device& device, const vk::Format colorFormat);
	static vk::PipelineLayout createPipelineLayout(const vk::Device& device);
	static vk::Pipeline createPipeline(const vk::Device& device, const char* vertPath, const char* fragPath, const vk::Extent2D &extents, const vk::RenderPass& renderPass, const vk::PipelineLayout& layout);

	Device& m_device;
public:
	vk::RenderPass m_renderPass;
	vk::PipelineLayout m_pipelineLayout;
	vk::Pipeline m_pipeline;
	RenderPipeline(Device &device, const char* vertPath, const char* fragPath, const vk::Extent2D &extents, const vk::Format &colorFormat);

	~RenderPipeline();
};

