#include "RenderPipeline.h"
#include <vector>
#include "Utils.h"
#include "Device.h"
#include "glm/vec2.hpp"

vk::ShaderModule RenderPipeline::createShaderModule(const vk::Device& device, const std::vector<char>& shader)
{
	const size_t size = shader.size();
	const uint32_t* c = reinterpret_cast<const uint32_t*>(shader.data());
	vk::ShaderModuleCreateInfo createInfo(vk::ShaderModuleCreateFlags(), size, c);

	return device.createShaderModule(createInfo);
}

vk::RenderPass RenderPipeline::createRenderPass(const vk::Device& device, vk::Format colorFormat)
{
	vk::AttachmentDescription color(vk::AttachmentDescriptionFlags(), colorFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

	vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, nullptr, colorAttachmentRef);

	vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite);

	vk::RenderPassCreateInfo renderPassInfo({}, color, subpass, dependency);
	return device.createRenderPass(renderPassInfo);;
}

vk::PipelineLayout RenderPipeline::createPipelineLayout(const vk::Device& device)
{
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, nullptr, nullptr);;
	return device.createPipelineLayout(pipelineLayoutInfo);
}

vk::Pipeline RenderPipeline::createPipeline(const vk::Device& device, const char* vertPath, const char* fragPath, const vk::Extent2D& extents, const vk::RenderPass& renderPass, const vk::PipelineLayout& layout)
{
	std::vector<char> vert = readFile(vertPath);
	std::vector<char> frag = readFile(fragPath);

	vk::ShaderModule vertShaderModule = createShaderModule(device, vert);
	vk::ShaderModule fragShaderModule = createShaderModule(device, frag);

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo({}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main");
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo({}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main");

	vk::VertexInputBindingDescription bindingDescription(0, sizeof(glm::vec2), vk::VertexInputRate::eVertex);
	vk::VertexInputAttributeDescription attributeDescription(0, 0, vk::Format::eR32G32Sfloat, 0);
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, bindingDescription, attributeDescription);

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::ePointList, VK_FALSE);

	vk::Viewport viewport(0, 0, extents.width, extents.height, 0.0f, 1.0f);
	vk::Rect2D scissor({ 0, 0 }, extents);
	vk::PipelineViewportStateCreateInfo viewportState({}, viewport, scissor);

	vk::PipelineRasterizationStateCreateInfo rasterizer({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise, VK_FALSE,
		0.0f, 0.0f, 0.0f, 1.0f);

	vk::PipelineMultisampleStateCreateInfo multisampling({}, vk::SampleCountFlagBits::e1, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE);
	
	vk::PipelineColorBlendAttachmentState colorState(VK_TRUE, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
		vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
	vk::PipelineColorBlendStateCreateInfo colorBlending({}, VK_FALSE, vk::LogicOp::eCopy, 1, &colorState, {0.0f, 0.0f, 0.0f, 0.0f});

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };
	vk::GraphicsPipelineCreateInfo pipelineCreateInfo(
		{},
		shaderStages,
		&vertexInputInfo,
		&inputAssembly,
		nullptr,
		&viewportState,
		&rasterizer,
		&multisampling,
		nullptr,
		&colorBlending,
		nullptr,
		layout,
		renderPass
	);

	vk::ResultValue<vk::Pipeline> result = device.createGraphicsPipeline(nullptr, pipelineCreateInfo);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vk::resultCheck(result.result, "Failed to create graphics pipeline");

	return result.value;
}

RenderPipeline::RenderPipeline(Device& device, const char* vertPath, const char* fragPath, const vk::Extent2D& extents, const vk::Format& colorFormat)
	: m_device(device),
	m_pipelineLayout(createPipelineLayout(device.m_device)),
	m_renderPass(createRenderPass(device.m_device, colorFormat)),
	m_pipeline(createPipeline(device.m_device, vertPath, fragPath, extents, m_renderPass, m_pipelineLayout))

{

}

RenderPipeline::~RenderPipeline()
{
	vkDestroyPipeline(m_device.m_device, m_pipeline, nullptr);
	vkDestroyPipelineLayout(m_device.m_device, m_pipelineLayout, nullptr);
	vkDestroyRenderPass(m_device.m_device, m_renderPass, nullptr);
}
