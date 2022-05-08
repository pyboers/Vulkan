#include <iostream>
#include "VulkanInstance.h"
#include "Window.h"
#include "ComputePipeline.h"
#include <chrono>
#include "Utils.h"
#include "vulkan/vulkan.hpp"
#include "glm/vec2.hpp"
#include <array>
using namespace std::chrono;

void recordCompute(const VulkanInstance &vkInstance, const vk::Buffer positions, const vk::DeviceSize size, const vk::Pipeline &computePipeline,
    const vk::PipelineLayout &pipelineLayout, const vk::DescriptorSet descriptorSet, const uint32_t elementCount) {

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

void recordRendering(const VulkanInstance& vkInstance, const uint32_t viewIndex, const vk::Buffer &positions, const vk::DeviceSize &size, const RenderPipeline &pipeline,
    const std::vector<vk::Framebuffer> &frameBuffers, const vk::Extent2D &extent, uint32_t elementCount) {

    std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 1.0f};
    std::vector<vk::ClearValue> clearValues = { vk::ClearValue(), vk::ClearValue() };
    clearValues[0].color = vk::ClearColorValue(clearColor);
    clearValues[1].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0.0f );

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
    vkInstance.m_device->m_primaryCommandBuffer.bindVertexBuffers(0, positions, vk::DeviceSize(0));
    vkInstance.m_device->m_primaryCommandBuffer.draw(elementCount, 1, 0, 0);

    vkInstance.m_device->m_primaryCommandBuffer.endRenderPass();
}

int main()
{
	std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	Window window = Window(1920, 1080, "Test App");
	VulkanInstance vkInstance = VulkanInstance(window, "Test App", 1, 0, 0, validationLayers);

    vk::DescriptorSetLayoutBinding descriptorSetLayoutBindings[2] = {
      vk::DescriptorSetLayoutBinding( 0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, 0),
      vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, 0)
    };

    vk::DescriptorSetLayoutCreateInfo dSetLayoutInfo({}, 2, descriptorSetLayoutBindings);

    vk::DescriptorSetLayout dLayout = vkInstance.m_device->m_device.createDescriptorSetLayout(dSetLayoutInfo);
	ComputePipeline test(*vkInstance.m_device, "shaders/comp.spv", dLayout);

    RenderPipeline pipeline = vkInstance.m_device->createRenderPipeline("shaders/vert.spv", "shaders/frag.spv");
    
    const uint32_t elementCount = 20000;
    const uint32_t size = elementCount * sizeof(glm::vec2); //It's in the name 
    vk::Buffer positions = vkInstance.m_device->createBuffer(size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer);
    vk::Buffer velocities = vkInstance.m_device->createBuffer(size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer);
    
    vk::MemoryRequirements positionRequirements = vkInstance.m_device->getBufferMemoryRequirements(positions);
    vk::MemoryRequirements velocityRequirements = vkInstance.m_device->getBufferMemoryRequirements(velocities);

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

    vk::MemoryAllocateInfo velocityMemAllocInfo(velocityRequirements.size, memTypeI);
    vk::DeviceMemory velocityMemory = vkInstance.m_device->allocateMemory(velocityMemAllocInfo);

    glm::vec2* positionBufferData = static_cast<glm::vec2*>(vkInstance.m_device->mapMemory(positionMemory, 0, positionRequirements.size));
    for (int i = 0; i < elementCount; i++) {
        positionBufferData[i] = glm::vec2(-1 + (randNorm() * 2), -1 + (randNorm() * 2));
    }
    vkInstance.m_device->unMapMemory(positionMemory);

    glm::vec2* velocityBufferData = static_cast<glm::vec2*>(vkInstance.m_device->mapMemory(velocityMemory, 0, velocityRequirements.size));
    for (int i = 0; i < elementCount; i++) {
        velocityBufferData[i] = glm::vec2(-1 + (randNorm() * 2), -1 + (randNorm() * 2)) * 0.01f;
        //velocityBufferData[i] = glm::vec2(0, 0);
    }
    vkInstance.m_device->unMapMemory(velocityMemory);
    vkInstance.m_device->bindBufferMemory(positions, positionMemory, 0);
    vkInstance.m_device->bindBufferMemory(velocities, velocityMemory, 0);

    vk::DescriptorPoolSize poolSize(vk::DescriptorType::eStorageBuffer, 2);

    vk::DescriptorPoolCreateInfo dPoolInfo({}, 1, 1, &poolSize);
    
    vk::DescriptorPool descriptorPool = vkInstance.m_device->m_device.createDescriptorPool(dPoolInfo);

    vk::DescriptorSetAllocateInfo allocInfo(descriptorPool, 1, &test.m_dSetLayout);

    vk::DescriptorSet descriptorSet = vkInstance.m_device->m_device.allocateDescriptorSets(allocInfo).front();

    vk::DescriptorBufferInfo positionBufferInfo(positions, 0, size);

    vk::DescriptorBufferInfo velocityBufferInfo(velocities, 0, size);

    vk::WriteDescriptorSet positionSet(descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &positionBufferInfo, nullptr);

    vk::WriteDescriptorSet velocitySet(descriptorSet, 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &velocityBufferInfo, nullptr);

    vkInstance.m_device->updateDescriptorSets(std::vector<vk::WriteDescriptorSet>({ positionSet, velocitySet }));

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

        vk::CommandBufferBeginInfo beginInfo;
        vkInstance.m_device->m_primaryCommandBuffer.begin(beginInfo);
        recordCompute(vkInstance, positions, size, test.m_pipeline, test.m_pipelineLayout, descriptorSet, elementCount);


        recordRendering(vkInstance, viewIndex, positions, size, pipeline, frameBuffers, extent, elementCount);


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

    vkDestroySemaphore(vkInstance.m_device->m_device, computeFinishedSemaphore, nullptr);
    vkDestroySemaphore(vkInstance.m_device->m_device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(vkInstance.m_device->m_device, renderFinishedSemaphore, nullptr);
    for (auto& framebuffer : frameBuffers) {
        vkDestroyFramebuffer(vkInstance.m_device->m_device, framebuffer, nullptr);
    }
    vkFreeMemory(vkInstance.m_device->m_device, positionMemory, nullptr);
    vkFreeMemory(vkInstance.m_device->m_device, velocityMemory, nullptr);
    vkDestroyBuffer(vkInstance.m_device->m_device, positions, nullptr);
    vkDestroyBuffer(vkInstance.m_device->m_device, velocities, nullptr);
    vkDestroyFence(vkInstance.m_device->m_device, fence, nullptr);
    vkDestroyDescriptorPool(vkInstance.m_device->m_device, descriptorPool, nullptr);
}
