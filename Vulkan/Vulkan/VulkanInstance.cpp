#include "VulkanInstance.h"
#include <stdexcept>
#include <iostream>

void VulkanInstance::validateValidationLayerSupport(std::vector<const char*> validationLayers) const
{
	std::vector<vk::LayerProperties> layerProperties = vk::enumerateInstanceLayerProperties();

	bool layersFound = true;
	for (const char* requested : validationLayers) {
		bool available = false;
		for (auto &layer : layerProperties) {
			if (strcmp(requested, layer.layerName) == 0) {
				available = true;
				break;
			}
		}
		layersFound &= available;
	}

	if (!layersFound) {
		throw std::runtime_error("Missing validation layer support!");
	}
}

vk::SurfaceKHR VulkanInstance::createSurface()
{
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(m_instance, m_window.m_window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface");
	}
	return vk::SurfaceKHR(surface);
}

vk::Instance VulkanInstance::createInstance(const char* appName, uint32_t majorVersion, uint32_t minorVersion, uint32_t patchVersion, std::vector<const char*> validationLayers)
{
	vk::ApplicationInfo appInfo(appName, 1, "Vlkan Engine", 1, VK_API_VERSION_1_0);

#ifdef NDEBUG
	std::vector<char*> layerNames;
#else
	validateValidationLayerSupport(validationLayers);
	std::vector<const char*> layerNames = validationLayers;
#endif

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions;
	extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

	vk::InstanceCreateInfo createInfo({}, &appInfo, layerNames, extensions);


	vk::Instance instance = vk::createInstance(createInfo);

	std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();
	
	std::cout << "Extensions available: " << std::endl;
	for (auto &extension : availableExtensions) {
		std::cout << '\t' << extension.extensionName << std::endl;
	}

	return instance;
}

VulkanInstance::VulkanInstance(Window& window, const char* appName, uint32_t majorVersion, uint32_t minorVersion, uint32_t patchVersion, std::vector<const char*> validationLayers) 
	: m_window(window),
	m_instance(createInstance(appName, majorVersion, minorVersion, patchVersion, validationLayers)),
	m_surface(createSurface()),
	m_device(new Device(*this))
{

}

VulkanInstance::~VulkanInstance()
{
	delete m_device; //Important this gets called before the next lines.
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}
