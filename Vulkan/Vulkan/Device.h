#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>
#include "SwapChain.h"
#include "RenderPipeline.h"
#include "vulkan/vulkan.hpp"

struct SwapChainSupportDetails {
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
};

struct QueueFamilies {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> computeFamily;
	std::optional<uint32_t> presentFamily;
};

class VulkanInstance;

/*
	Currently just fetches a discrete gpu and throws exception if not enough support.
*/
class Device
{
private:
	static vk::PhysicalDevice fetchPhysicalDeviceAndSetFamilies(const VulkanInstance& instance);

	static QueueFamilies fetchQueueFamilies(const vk::PhysicalDevice& device, const VulkanInstance &instance);

    static bool confirmSwapChainSupport(const vk::PhysicalDevice &device, const VulkanInstance& instance);

	static bool confirmDeviceExtensionSupport(const vk::PhysicalDevice& device);
	
	vk::Device fetchDevice() const;




public:
	static SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice &device, const VulkanInstance& instance);

	VulkanInstance& m_instance;
	vk::PhysicalDevice m_physicalDevice;
	SwapChainSupportDetails m_swapChainSupportDetails;

	QueueFamilies m_queueFamilies;

	vk::Device m_device;

	vk::Queue m_graphicsQueue;
	vk::Queue m_presentQueue;
	vk::Queue m_computeQueue;

	vk::CommandPool m_oneTimeCommandPool;
	vk::CommandPool m_primaryCommandPool;
	vk::CommandBuffer m_primaryCommandBuffer;

	SwapChain* m_swapChain;

	Device(VulkanInstance &instance);


	RenderPipeline createRenderPipeline(const char* vertPath, const char* fragPath);

	vk::Buffer createBuffer(uint32_t size, const vk::Flags<vk::BufferUsageFlagBits> usage) const;

	vk::MemoryRequirements getBufferMemoryRequirements(const vk::Buffer &buffer) const;

	vk::PhysicalDeviceMemoryProperties getMemoryProperties() const;

	vk::DeviceMemory allocateMemory(const vk::MemoryAllocateInfo &info) const;

	void * mapMemory(const vk::DeviceMemory &memory, const vk::DeviceSize offset, const vk::DeviceSize size) const;
	void unMapMemory(const vk::DeviceMemory memory) const;

	void bindBufferMemory(const vk::Buffer & buffer, const vk::DeviceMemory &memory, const vk::DeviceSize offset) const;

	void updateDescriptorSets(const std::vector<vk::WriteDescriptorSet> &sets) const;

	vk::CommandBuffer &beginSingleUseCommands() const;

	void endSingleUseCommands(vk::CommandBuffer &commandBuffer) const;

	~Device();

};

