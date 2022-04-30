#include "VulkanInstance.h"
#include <stdexcept>
#include <iostream>

void VulkanInstance::validateValidationLayerSupport(std::vector<const char*> validationLayers) const
{
	uint32_t layercount;
	vkEnumerateInstanceLayerProperties(&layercount, nullptr);

	VkLayerProperties* layers = new VkLayerProperties[layercount];
	vkEnumerateInstanceLayerProperties(&layercount, layers);

	bool layersFound = true;
	for (const char* requested : validationLayers) {
		bool available = false;
		for (uint32_t i = 0; i < layercount; i++) {
			if (strcmp(requested, layers[i].layerName) == 0) {
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

VulkanInstance::VulkanInstance(const char* appName, uint32_t majorVersion, uint32_t minorVersion, uint32_t patchVersion, std::vector<const char*> validationLayers)
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName;
	appInfo.applicationVersion = VK_MAKE_VERSION(majorVersion, minorVersion, patchVersion);
	appInfo.pEngineName = "Vulkan Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

#ifdef NDEBUG
	createInfo.enabledLayerCount = 0;
#else
	validateValidationLayerSupport(validationLayers);
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();
#endif

	if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan instance!");
	}

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	VkExtensionProperties * extensions = new VkExtensionProperties[extensionCount];
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);

	std::cout << "Extensions available: " << std::endl;
	for (uint32_t i = 0; i < extensionCount; i++) {
		std::cout << '\t' << extensions[i].extensionName << std::endl;
	}
}

VulkanInstance::~VulkanInstance()
{
	vkDestroyInstance(m_instance, nullptr);
}
