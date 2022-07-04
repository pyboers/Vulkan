#include "Device.h"
#include <iostream>
#include <set>
#include "VulkanInstance.h"

SwapChainSupportDetails Device::querySwapChainSupport(const vk::PhysicalDevice& device, const VulkanInstance& instance)
{
    SwapChainSupportDetails details;

    details.capabilities = device.getSurfaceCapabilitiesKHR(instance.m_surface);

    details.formats = device.getSurfaceFormatsKHR(instance.m_surface);

    details.presentModes = device.getSurfacePresentModesKHR(instance.m_surface);
    return details;
}

RenderPipeline Device::createRenderPipeline(const char* vertPath, const char* fragPath)
{
    return RenderPipeline(*this, vertPath, fragPath, m_swapChain->m_swapChainExtent, m_swapChain->m_surfaceFormat.format);
}

vk::Buffer Device::createBuffer(uint32_t size, const vk::Flags<vk::BufferUsageFlagBits> usage) const
{

    vk::BufferCreateInfo createInfo(vk::BufferCreateFlags(), size, usage, vk::SharingMode::eExclusive
        , { m_queueFamilies.computeFamily.value() });

    return m_device.createBuffer(createInfo);

}

vk::MemoryRequirements Device::getBufferMemoryRequirements(const vk::Buffer &buffer) const
{
    return m_device.getBufferMemoryRequirements(buffer);
}

vk::PhysicalDeviceMemoryProperties Device::getMemoryProperties() const
{
    return m_physicalDevice.getMemoryProperties();
}

vk::DeviceMemory Device::allocateMemory(const vk::MemoryAllocateInfo &info) const
{
    return m_device.allocateMemory(info);
}

void* Device::mapMemory(const vk::DeviceMemory &memory, const vk::DeviceSize offset, const vk::DeviceSize size) const
{
    return m_device.mapMemory(memory, offset, size);
}

void Device::unMapMemory(const vk::DeviceMemory memory) const
{
    m_device.unmapMemory(memory);
}

void Device::bindBufferMemory(const vk::Buffer &buffer, const vk::DeviceMemory &memory, const vk::DeviceSize offset) const
{
    m_device.bindBufferMemory(buffer, memory, offset);
}

void Device::updateDescriptorSets(const std::vector<vk::WriteDescriptorSet> &sets) const
{
    m_device.updateDescriptorSets(sets, nullptr);
}

vk::CommandBuffer Device::beginSingleUseCommands() const
{

    vk::CommandBuffer commandBuffer(m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_oneTimeCommandPool, vk::CommandBufferLevel::ePrimary, 1)).front());

    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void Device::endSingleUseCommands(vk::CommandBuffer& commandBuffer) const
{
    commandBuffer.end();

    vk::SubmitInfo info({}, {}, commandBuffer, {});

    m_graphicsQueue.submit(info);

    m_graphicsQueue.waitIdle();

    m_device.freeCommandBuffers(m_oneTimeCommandPool, commandBuffer);
}

bool Device::confirmSwapChainSupport(const vk::PhysicalDevice & device, const VulkanInstance& instance)
{
    SwapChainSupportDetails details = querySwapChainSupport(device, instance);
    return !details.formats.empty() && !details.presentModes.empty();
}

bool Device::confirmDeviceExtensionSupport(const vk::PhysicalDevice &device)
{
    
    std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilies Device::fetchQueueFamilies(const vk::PhysicalDevice &device, const VulkanInstance& instance)
{
    std::vector<vk::QueueFamilyProperties> families = device.getQueueFamilyProperties();

    QueueFamilies queueFamilies;

    bool foundFamilyBoth = false; //Stop searching for graphics queue if found presenting and graphics.
    for (uint32_t i = 0; i < families.size(); i++) {
        vk::Bool32 supportsPresent = device.getSurfaceSupportKHR(i, instance.m_surface);

        if (supportsPresent && !foundFamilyBoth) {
            queueFamilies.presentFamily = i;
        }
        if (families[i].queueFlags & vk::QueueFlagBits::eGraphics && !foundFamilyBoth) {
            queueFamilies.graphicsFamily = i;
            if (supportsPresent) {
                foundFamilyBoth = true;
            }
        }
        if (families[i].queueFlags & vk::QueueFlagBits::eCompute) {
            if(queueFamilies.graphicsFamily.has_value() && (!queueFamilies.computeFamily.has_value() || queueFamilies.graphicsFamily.value() != queueFamilies.computeFamily.value()))
                queueFamilies.computeFamily = i;
        }

    }
    if (!queueFamilies.graphicsFamily.has_value() || !queueFamilies.presentFamily.has_value()
        || !queueFamilies.computeFamily.has_value())
    {
        throw std::runtime_error("Couldn't find required queues!");
    }

    if (queueFamilies.graphicsFamily.value() != queueFamilies.computeFamily.value()) {
        throw std::runtime_error("Couldn't create primary queue with both graphics and compute");
    }

    return queueFamilies;
}

vk::PhysicalDevice Device::fetchPhysicalDeviceAndSetFamilies(const VulkanInstance& instance)
{
    std::vector<vk::PhysicalDevice> devices = instance.m_instance.enumeratePhysicalDevices();

    vk::PhysicalDevice* selectedDevice = nullptr;
    std::cout << "Chosen Device: " << std::endl;
    for (auto& device : devices) {
        vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
        vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) { //Just going straight for my gpu for now.
            selectedDevice = &device;
            std::cout << '\t' << deviceProperties.deviceName << std::endl;
            break;
        }
        else if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
            selectedDevice = &device;
            std::cout << "\t" << deviceProperties.deviceName << std::endl;
        }

    }

    if (!selectedDevice) {
        throw std::runtime_error("Couldn't find a compatible device");
    }

    if (!confirmDeviceExtensionSupport(*selectedDevice) || !confirmSwapChainSupport(*selectedDevice, instance)) {
        throw std::runtime_error("Didn't find available gpu");
    }

    fetchQueueFamilies(*selectedDevice, instance);

    return *selectedDevice;
}

vk::Device Device::fetchDevice() const {
    std::vector<vk::DeviceQueueCreateInfo> queueInfos;
    std::set<uint32_t> familySet = { m_queueFamilies.graphicsFamily.value(), m_queueFamilies.presentFamily.value(), m_queueFamilies.computeFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t family : familySet) {
        vk::DeviceQueueCreateInfo queueInfo(vk::DeviceQueueCreateFlags(), family, 1, &queuePriority);
        queueInfos.push_back(queueInfo);
    }

    std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    vk::DeviceCreateInfo createInfo(vk::DeviceCreateFlags(), queueInfos, {}, requiredExtensions);

    return m_physicalDevice.createDevice(createInfo);
}

Device::Device(VulkanInstance& instance)
    : m_instance(instance), 
    m_physicalDevice(fetchPhysicalDeviceAndSetFamilies(m_instance)),
    m_queueFamilies(fetchQueueFamilies(m_physicalDevice, m_instance)),
    m_swapChainSupportDetails(querySwapChainSupport(m_physicalDevice, m_instance)),
    m_device(fetchDevice()),
    m_graphicsQueue(m_device.getQueue(m_queueFamilies.graphicsFamily.value(), 0)),
    m_presentQueue(m_device.getQueue(m_queueFamilies.presentFamily.value(), 0)),
    m_computeQueue(m_device.getQueue(m_queueFamilies.computeFamily.value(), 0)),
    m_swapChain(new SwapChain(*this)),
    m_oneTimeCommandPool(m_device.createCommandPool(vk::CommandPoolCreateInfo({ vk::CommandPoolCreateFlagBits::eResetCommandBuffer }, m_queueFamilies.graphicsFamily.value()))),
    m_primaryCommandPool(m_device.createCommandPool(vk::CommandPoolCreateInfo({ vk::CommandPoolCreateFlagBits::eResetCommandBuffer }, m_queueFamilies.graphicsFamily.value()))),
    m_primaryCommandBuffer(m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_primaryCommandPool, vk::CommandBufferLevel::ePrimary, 1)).front())

{

}

Device::~Device()
{
    vkDestroyCommandPool(m_device, m_primaryCommandPool, nullptr);
    delete m_swapChain;
    vkDestroyDevice(m_device, nullptr);
}
