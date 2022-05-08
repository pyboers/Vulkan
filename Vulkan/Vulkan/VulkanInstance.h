#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Window.h"
#include "Device.h"
#include <memory>
#include "vulkan/vulkan.hpp"

class VulkanInstance {
private:

	void validateValidationLayerSupport(std::vector<const char*> validationLayers) const;
	vk::SurfaceKHR createSurface();
	vk::Instance createInstance(const char* appName, uint32_t majorVersion, uint32_t minorVersion, uint32_t patchVersion, std::vector<const char*> validationLayers);

	//No copy constructor due to unique pointer.
public:
	Window& m_window; //Using the instance with only one window for now, since the required extensions are being taken from glfw.
	vk::Instance m_instance;
	vk::SurfaceKHR m_surface;
	Device *m_device;
	
	VulkanInstance(Window& window, const char* appName, uint32_t majorVersion, uint32_t minorVersion, uint32_t patchVersion, std::vector<const char*> validationLayers);

	~VulkanInstance();
};
