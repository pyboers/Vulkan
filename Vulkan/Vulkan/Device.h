#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>
#include "SwapChain.h"
#include "RenderPipeline.h"

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
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

	const std::vector<const char*> m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };


    bool confirmSwapChainSupport(VkPhysicalDevice device);

	bool confirmDeviceExtensionSupport(VkPhysicalDevice device);

	void setupQueueFamilies(VkPhysicalDevice device);
public:
	VulkanInstance& m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;

	QueueFamilies m_queueFamilies;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	VkQueue m_computeQueue;

	SwapChain* m_swapChain;

	Device(VulkanInstance &instance);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	RenderPipeline createRenderPipeline(const char* vertPath, const char* fragPath);

	~Device();

};

