#include "Device.h"
#include <iostream>
#include <set>
#include "VulkanInstance.h"

SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_instance.m_surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_instance.m_surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_instance.m_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_instance.m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_instance.m_surface, &presentModeCount, details.presentModes.data());
    }
    return details;
}

bool Device::confirmSwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details = querySwapChainSupport(device);
    return !details.formats.empty() && !details.presentModes.empty();
}

bool Device::confirmDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void Device::setupQueueFamilies(VkPhysicalDevice device)
{
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

    VkQueueFamilyProperties* families = new VkQueueFamilyProperties[count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families);

    for (uint32_t i = 0; i < count; i++) {
        VkBool32 supportsPresent = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_instance.m_surface, &supportsPresent);
        if (supportsPresent) {
            m_queueFamilies.presentFamily = i;
        }
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_queueFamilies.graphicsFamily = i;
            if (supportsPresent) {
                break; //Rather use the same queue if possible.
            }
        }
    }
    if (!m_queueFamilies.graphicsFamily.has_value() || !m_queueFamilies.presentFamily.has_value())
    {
        throw std::runtime_error("Couldn't find required queues!");
    }
}

Device::Device(VulkanInstance& instance): m_instance(instance)
{
    m_physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance.m_instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("No device with Vulkun support found!");
    }
    VkPhysicalDevice* devices = new VkPhysicalDevice[deviceCount];
    vkEnumeratePhysicalDevices(m_instance.m_instance, &deviceCount, devices);

    std::cout << "Chosen Device: " << std::endl;
    for (uint32_t i = 0; i < deviceCount; i++) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(devices[i], &deviceFeatures);
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { //Just going straight for my gpu for now.
            m_physicalDevice = devices[i];
            std::cout << '\t' << deviceProperties.deviceName << std::endl;
            break;
        }

    }

    if (m_physicalDevice == VK_NULL_HANDLE || !confirmDeviceExtensionSupport(m_physicalDevice) || !confirmSwapChainSupport(m_physicalDevice)) {
        throw std::runtime_error("Didn't find available discrete gpu");
    }

    setupQueueFamilies(m_physicalDevice);
}

Device::~Device()
{
    vkDestroyDevice(m_device, nullptr);
}
