#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class VulkanInstance {
private:
	VkInstance m_instance;

	void validateValidationLayerSupport(std::vector<const char*> validationLayers) const;


public:
	VulkanInstance(const char* appName, uint32_t majorVersion, uint32_t minorVersion, uint32_t patchVersion, std::vector<const char*> validationLayers);

	~VulkanInstance();
};
