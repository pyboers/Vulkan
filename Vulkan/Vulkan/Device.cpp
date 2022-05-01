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

RenderPipeline Device::createRenderPipeline(const char* vertPath, const char* fragPath)
{
    return RenderPipeline(*this, vertPath, fragPath, m_swapChain->m_swapChainExtent, m_swapChain->m_swapChainImageFormat);
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

    std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

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

    bool foundFamilyBoth = false; //Stop searching for graphics queue if found presenting and graphics.
    for (uint32_t i = 0; i < count; i++) {
        VkBool32 supportsPresent = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_instance.m_surface, &supportsPresent);
        if (supportsPresent && !foundFamilyBoth) {
            m_queueFamilies.presentFamily = i;
        }
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && !foundFamilyBoth) {
            m_queueFamilies.graphicsFamily = i;
            if (supportsPresent) {
                foundFamilyBoth = true;
            }
        }
        if (families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            m_queueFamilies.computeFamily = i;
        }

    }
    if (!m_queueFamilies.graphicsFamily.has_value() || !m_queueFamilies.presentFamily.has_value()
        || !m_queueFamilies.computeFamily.has_value())
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

    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    std::set<uint32_t> familySet = { m_queueFamilies.graphicsFamily.value(), m_queueFamilies.presentFamily.value(), m_queueFamilies.computeFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t family : familySet) {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = family;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;
        queueInfos.push_back(queueInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();


    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }

    vkGetDeviceQueue(m_device, m_queueFamilies.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_queueFamilies.presentFamily.value(), 0, &m_presentQueue);
    vkGetDeviceQueue(m_device, m_queueFamilies.computeFamily.value(), 0, &m_computeQueue);

    m_swapChain = new SwapChain(*this);
}

Device::~Device()
{
    delete m_swapChain;
    vkDestroyDevice(m_device, nullptr);
}
