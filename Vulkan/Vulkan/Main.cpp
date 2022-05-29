#include <iostream>
#include "VulkanInstance.h"
#include "Window.h"
#include "ComputePipeline.h"
#include <chrono>
#include "Utils.h"
#include "vulkan/vulkan.hpp"
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include <array>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#define TYPECOUNT 20
using namespace std::chrono;

void ImGuiVkCheck(VkResult result) {
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed vulkan call in ImGui");
	}
}


void setupImGui(GLFWwindow* window, VkInstance instance, Device &device, VkPhysicalDevice physicalDevice, uint32_t queueFamily, VkQueue queue, 
	uint32_t minImageCount, uint32_t imageCount) {

	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	VkDescriptorPool descriptorPool;
	if (vkCreateDescriptorPool(device.m_device, &pool_info, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Couldn't create ImGui descriptor pool.");
	};

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();


	ImGui_ImplGlfw_InitForVulkan(window, true);

	ImGui_ImplVulkan_InitInfo info = {};
	info.Instance = instance;
	info.PhysicalDevice = physicalDevice;
	info.Device = device.m_device;
	info.QueueFamily = queueFamily;
	info.Queue = queue;
	info.PipelineCache = VK_NULL_HANDLE;
	info.DescriptorPool = descriptorPool;
	info.Allocator = nullptr;
	info.MinImageCount = minImageCount;
	info.ImageCount = imageCount;
	info.CheckVkResultFn = ImGuiVkCheck;

	VkAttachmentDescription attachment = {};
	attachment.format = (VkFormat)device.m_swapChain->m_surfaceFormat.format;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment = {};
	color_attachment.attachment = 0;
	color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &attachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkRenderPass imGuiRenderPass;
	if (vkCreateRenderPass(device.m_device, &renderPassInfo, nullptr, &imGuiRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Could not create Dear ImGui's render pass");
	}

	ImGui_ImplVulkan_Init(&info, imGuiRenderPass);
	
	vk::CommandBuffer& commandBuffer = device.beginSingleUseCommands();
	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
	//device.endSingleUseCommands(commandBuffer);
	commandBuffer.end();
	vk::SubmitInfo submitinfo({}, {}, commandBuffer, {});

	device.m_graphicsQueue.submit(submitinfo);

	device.m_graphicsQueue.waitIdle();

	device.m_device.freeCommandBuffers(device.m_oneTimeCommandPool, 1, &commandBuffer);
	std::cout << "Reached here";


}





struct ParticleBehaviour {
	glm::vec2 behaviours[TYPECOUNT];
};

void recordCompute(const VulkanInstance& vkInstance, const vk::Buffer positions, const vk::DeviceSize size, const vk::Pipeline& computePipeline,
	const vk::PipelineLayout& pipelineLayout, const vk::DescriptorSet descriptorSet, const uint32_t elementCount) {

	vk::BufferMemoryBarrier memoryBarrierCompute(vk::AccessFlagBits::eNone, vk::AccessFlagBits::eShaderWrite, vkInstance.m_device->m_queueFamilies.graphicsFamily.value(),
		vkInstance.m_device->m_queueFamilies.computeFamily.value(), positions, 0, size);

	vkInstance.m_device->m_primaryCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eVertexInput, vk::PipelineStageFlagBits::eComputeShader,
		{}, 0, nullptr, 1, &memoryBarrierCompute, 0, nullptr);


	vkInstance.m_device->m_primaryCommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline);

	vkInstance.m_device->m_primaryCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, descriptorSet, nullptr);

	vkInstance.m_device->m_primaryCommandBuffer.dispatch((elementCount / 64) + 1, 1, 1);

	vk::BufferMemoryBarrier memoryBarrierVertex(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eNone, vkInstance.m_device->m_queueFamilies.computeFamily.value(),
		vkInstance.m_device->m_queueFamilies.graphicsFamily.value(), positions, 0, size);

	vkInstance.m_device->m_primaryCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eVertexInput,
		{}, 0, nullptr, 1, &memoryBarrierVertex, 0, nullptr);
}

void recordRendering(const VulkanInstance& vkInstance, const uint32_t viewIndex, const vk::Buffer& positions, const vk::DeviceSize& size, const vk::Buffer& colors, const RenderPipeline& pipeline,
	const std::vector<vk::Framebuffer>& frameBuffers, const vk::Extent2D& extent, uint32_t elementCount) {

	std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	std::vector<vk::ClearValue> clearValues = { vk::ClearValue(), vk::ClearValue() };
	clearValues[0].color = vk::ClearColorValue(clearColor);
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0.0f);

	vk::BufferMemoryBarrier memoryBarrierCompute(vk::AccessFlagBits::eNone, vk::AccessFlagBits::eVertexAttributeRead, vkInstance.m_device->m_queueFamilies.computeFamily.value(),
		vkInstance.m_device->m_queueFamilies.graphicsFamily.value(), positions, 0, size);

	vkInstance.m_device->m_primaryCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eVertexInput,
		{}, 0, nullptr, 1, &memoryBarrierCompute, 0, nullptr);


	vk::RenderPassBeginInfo renderInfo(pipeline.m_renderPass, frameBuffers[viewIndex], vk::Rect2D({ 0, 0 }, extent), clearValues);
	vkInstance.m_device->m_primaryCommandBuffer.beginRenderPass(renderInfo, vk::SubpassContents::eInline);

	vk::Viewport viewport(0.0f, 0.1f, extent.width, extent.height);
	vkInstance.m_device->m_primaryCommandBuffer.setViewport(0, viewport);

	vk::Rect2D scissor({ 0, 0 }, extent);
	vkInstance.m_device->m_primaryCommandBuffer.setScissor(0, scissor);

	vkInstance.m_device->m_primaryCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.m_pipeline);
	vkInstance.m_device->m_primaryCommandBuffer.bindVertexBuffers(0, { positions, colors, }, { vk::DeviceSize(0), vk::DeviceSize(0) });
	vkInstance.m_device->m_primaryCommandBuffer.draw(elementCount, 1, 0, 0);


	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vkInstance.m_device->m_primaryCommandBuffer);

	vkInstance.m_device->m_primaryCommandBuffer.endRenderPass();
}

int main()
{
	srand(time(0));
	std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	Window window = Window(1920, 1080, "Test App");
	VulkanInstance vkInstance = VulkanInstance(window, "Test App", 1, 0, 0, validationLayers);

	vk::DescriptorSetLayoutBinding descriptorSetLayoutBindings[3] = {
	  vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, 0),
	  vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, 0),
	  vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, 0)
	};

	vk::DescriptorSetLayoutCreateInfo dSetLayoutInfo({}, 3, descriptorSetLayoutBindings);

	vk::DescriptorSetLayout dLayout = vkInstance.m_device->m_device.createDescriptorSetLayout(dSetLayoutInfo);
	ComputePipeline test(*vkInstance.m_device, "shaders/comp.spv", dLayout);

	RenderPipeline pipeline = vkInstance.m_device->createRenderPipeline("shaders/vert.spv", "shaders/frag.spv");

	setupImGui(window.m_window, vkInstance.m_instance, *vkInstance.m_device, vkInstance.m_device->m_physicalDevice, vkInstance.m_device->m_queueFamilies.graphicsFamily.value(),
		vkInstance.m_device->m_graphicsQueue, vkInstance.m_device->m_swapChainSupportDetails.capabilities.minImageCount, vkInstance.m_device->m_swapChainSupportDetails.capabilities.minImageCount + 1);

	const uint32_t elementCount = 20000;
	vk::Buffer positions = vkInstance.m_device->createBuffer(elementCount * sizeof(glm::vec2), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer);
	vk::Buffer colors = vkInstance.m_device->createBuffer(elementCount * sizeof(glm::vec4), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer);
	vk::Buffer velocities = vkInstance.m_device->createBuffer(elementCount * sizeof(glm::vec2), vk::BufferUsageFlagBits::eStorageBuffer);
	vk::Buffer behaviours = vkInstance.m_device->createBuffer(TYPECOUNT * sizeof(ParticleBehaviour), vk::BufferUsageFlagBits::eStorageBuffer);

	vk::MemoryRequirements positionRequirements = vkInstance.m_device->getBufferMemoryRequirements(positions);
	vk::MemoryRequirements colorRequirements = vkInstance.m_device->getBufferMemoryRequirements(colors);
	vk::MemoryRequirements velocityRequirements = vkInstance.m_device->getBufferMemoryRequirements(velocities);
	vk::MemoryRequirements behaviourRequirements = vkInstance.m_device->getBufferMemoryRequirements(behaviours);

	vk::PhysicalDeviceMemoryProperties memProps = vkInstance.m_device->getMemoryProperties();

	uint32_t memTypeI = 1;
	vk::DeviceSize memHeapSize = 0;

	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
	{
		vk::MemoryType memType = memProps.memoryTypes[i];
		if ((vk::MemoryPropertyFlagBits::eHostVisible & memType.propertyFlags) &&
			(vk::MemoryPropertyFlagBits::eHostCoherent & memType.propertyFlags))
		{
			memHeapSize = memProps.memoryHeaps[memType.heapIndex].size;
			memTypeI = i;
			break;
		}
	}

	std::cout << "Fetched memory heap size: " << memHeapSize / (1024 * 1024) << " MB" << std::endl;

	vk::MemoryAllocateInfo positionMemAllocInfo(positionRequirements.size, memTypeI);
	VkDeviceMemory positionMemory = vkInstance.m_device->allocateMemory(positionMemAllocInfo);

	vk::MemoryAllocateInfo colorMemAllocInfo(colorRequirements.size, memTypeI);
	VkDeviceMemory colorMemory = vkInstance.m_device->allocateMemory(colorMemAllocInfo);

	vk::MemoryAllocateInfo velocityMemAllocInfo(velocityRequirements.size, memTypeI);
	vk::DeviceMemory velocityMemory = vkInstance.m_device->allocateMemory(velocityMemAllocInfo);

	vk::MemoryAllocateInfo behaviourMemAllocInfo(behaviourRequirements.size, memTypeI);
	vk::DeviceMemory behaviourMemory = vkInstance.m_device->allocateMemory(behaviourMemAllocInfo);

	std::vector<glm::vec4> typeColors;
	typeColors.reserve(TYPECOUNT);
	for (int i = 0; i < TYPECOUNT; i++) {
		typeColors.push_back(glm::vec4(randNorm(0.0f, 1.0f), randNorm(0.0f, 1.0f), randNorm(0.0f, 1.0f), 0.5f));
	}

	glm::vec2* positionBufferData = static_cast<glm::vec2*>(vkInstance.m_device->mapMemory(positionMemory, 0, positionRequirements.size));
	for (int i = 0; i < elementCount; i++) {
		positionBufferData[i] = glm::vec2(randNorm(-0.9f, 0.9f), randNorm(-0.5f, 0.5f));
	}
	vkInstance.m_device->unMapMemory(positionMemory);

	glm::vec4* colorBufferData = static_cast<glm::vec4*>(vkInstance.m_device->mapMemory(colorMemory, 0, colorRequirements.size));
	for (int i = 0; i < elementCount; i++) {
		colorBufferData[i] = typeColors[i / (elementCount / TYPECOUNT)];
	}
	vkInstance.m_device->unMapMemory(colorMemory);

	glm::vec2* velocityBufferData = static_cast<glm::vec2*>(vkInstance.m_device->mapMemory(velocityMemory, 0, velocityRequirements.size));
	for (int i = 0; i < elementCount; i++) {
		//velocityBufferData[i] = glm::vec2(-1 + (randNorm() * 2), -1 + (randNorm() * 2)) * 0.01f;
		velocityBufferData[i] = glm::vec2(0, 0);
	}
	vkInstance.m_device->unMapMemory(velocityMemory);

	const float maxVelocity = 0.05;
	ParticleBehaviour* behaviourBufferData = static_cast<ParticleBehaviour*>(vkInstance.m_device->mapMemory(behaviourMemory, 0, behaviourRequirements.size));
	for (int i = 0; i < TYPECOUNT; i++) {
		for (int j = 0; j < TYPECOUNT; j++) {
			behaviourBufferData[i].behaviours[j] = glm::vec2(randNorm(0.01f, 0.05f), randNorm(-1.0f, 1.0f) * maxVelocity);
		}
	}
	vkInstance.m_device->unMapMemory(behaviourMemory);

	vkInstance.m_device->bindBufferMemory(positions, positionMemory, 0);
	vkInstance.m_device->bindBufferMemory(colors, colorMemory, 0);
	vkInstance.m_device->bindBufferMemory(velocities, velocityMemory, 0);
	vkInstance.m_device->bindBufferMemory(behaviours, behaviourMemory, 0);

	vk::DescriptorPoolSize poolSize(vk::DescriptorType::eStorageBuffer, 3);

	vk::DescriptorPoolCreateInfo dPoolInfo({}, 1, 1, &poolSize);

	vk::DescriptorPool descriptorPool = vkInstance.m_device->m_device.createDescriptorPool(dPoolInfo);

	vk::DescriptorSetAllocateInfo allocInfo(descriptorPool, 1, &test.m_dSetLayout);

	vk::DescriptorSet descriptorSet = vkInstance.m_device->m_device.allocateDescriptorSets(allocInfo).front();

	vk::DescriptorBufferInfo positionBufferInfo(positions, 0, elementCount * sizeof(glm::vec2));

	vk::DescriptorBufferInfo velocityBufferInfo(velocities, 0, elementCount * sizeof(glm::vec2));

	vk::DescriptorBufferInfo behaviourBufferInfo(behaviours, 0, TYPECOUNT * sizeof(ParticleBehaviour));

	vk::WriteDescriptorSet positionSet(descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &positionBufferInfo, nullptr);

	vk::WriteDescriptorSet velocitySet(descriptorSet, 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &velocityBufferInfo, nullptr);

	vk::WriteDescriptorSet behaviourSet(descriptorSet, 2, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &behaviourBufferInfo, nullptr);

	vkInstance.m_device->updateDescriptorSets(std::vector<vk::WriteDescriptorSet>({ positionSet, velocitySet, behaviourSet }));

	const std::vector<vk::ImageView>& imageViews = vkInstance.m_device->m_swapChain->m_swapChainImageViews;
	const vk::Extent2D& extent = vkInstance.m_device->m_swapChain->m_swapChainExtent;
	std::vector<vk::Framebuffer> frameBuffers;
	frameBuffers.reserve(imageViews.size());
	for (auto& imageView : imageViews) {
		vk::FramebufferCreateInfo createInfo({}, pipeline.m_renderPass, imageView, extent.width, extent.height, 1);
		frameBuffers.push_back(vkInstance.m_device->m_device.createFramebuffer(createInfo));
	}

	vk::SemaphoreCreateInfo semaphoreInfo;

	vk::Semaphore computeFinishedSemaphore = vkInstance.m_device->m_device.createSemaphore(semaphoreInfo);
	vk::Semaphore imageAvailableSemaphore = vkInstance.m_device->m_device.createSemaphore(semaphoreInfo);
	vk::Semaphore renderFinishedSemaphore = vkInstance.m_device->m_device.createSemaphore(semaphoreInfo);




	vk::FenceCreateInfo fenceInfo;

	vk::Fence fence = vkInstance.m_device->m_device.createFence(fenceInfo);

	while (!glfwWindowShouldClose(window.m_window)) {
		const uint32_t viewIndex = vkInstance.m_device->m_device.acquireNextImageKHR(vkInstance.m_device->m_swapChain->m_swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE).value;

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();
		ImGui::Render();

		vk::CommandBufferBeginInfo beginInfo;
		vkInstance.m_device->m_primaryCommandBuffer.begin(beginInfo);
		recordCompute(vkInstance, positions, elementCount * sizeof(glm::vec2), test.m_pipeline, test.m_pipelineLayout, descriptorSet, elementCount);


		recordRendering(vkInstance, viewIndex, positions, elementCount * sizeof(glm::vec2), colors, pipeline, frameBuffers, extent, elementCount);

		vkInstance.m_device->m_primaryCommandBuffer.end();


		std::vector<vk::PipelineStageFlags> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		vk::SubmitInfo submitInfo(imageAvailableSemaphore, waitStages, vkInstance.m_device->m_primaryCommandBuffer, renderFinishedSemaphore);
		vkInstance.m_device->m_graphicsQueue.submit(submitInfo, fence);

		vk::PresentInfoKHR presentInfo(renderFinishedSemaphore, vkInstance.m_device->m_swapChain->m_swapChain, viewIndex, nullptr);
		vkInstance.m_device->m_presentQueue.presentKHR(presentInfo);

		vkInstance.m_device->m_device.waitForFences(fence, VK_TRUE, UINT_MAX);

		vkInstance.m_device->m_device.resetFences(fence);
		glfwPollEvents();
	}

	std::cout << "Start destroying" << std::endl;

	vkDestroySemaphore(vkInstance.m_device->m_device, computeFinishedSemaphore, nullptr);
	vkDestroySemaphore(vkInstance.m_device->m_device, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(vkInstance.m_device->m_device, renderFinishedSemaphore, nullptr);
	for (auto& framebuffer : frameBuffers) {
		vkDestroyFramebuffer(vkInstance.m_device->m_device, framebuffer, nullptr);
	}
	vkFreeMemory(vkInstance.m_device->m_device, positionMemory, nullptr);
	vkFreeMemory(vkInstance.m_device->m_device, colorMemory, nullptr);
	vkFreeMemory(vkInstance.m_device->m_device, velocityMemory, nullptr);
	vkFreeMemory(vkInstance.m_device->m_device, behaviourMemory, nullptr);
	vkDestroyBuffer(vkInstance.m_device->m_device, positions, nullptr);
	vkDestroyBuffer(vkInstance.m_device->m_device, colors, nullptr);
	vkDestroyBuffer(vkInstance.m_device->m_device, velocities, nullptr);
	vkDestroyBuffer(vkInstance.m_device->m_device, behaviours, nullptr);
	vkDestroyFence(vkInstance.m_device->m_device, fence, nullptr);
	vkDestroyDescriptorPool(vkInstance.m_device->m_device, descriptorPool, nullptr);

}
