#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilies {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
};

class VulkanInstance;

/*
	Currently just fetches a discrete gpu and throws exception if not enough support.
*/
class Device
{
private:
	VulkanInstance& m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;

	QueueFamilies m_queueFamilies;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    bool confirmSwapChainSupport(VkPhysicalDevice device);

	bool confirmDeviceExtensionSupport(VkPhysicalDevice device);

	void setupQueueFamilies(VkPhysicalDevice device);
public:
	Device(VulkanInstance &instance);

	~Device();

};

