#include "SwapChain.h"
#include "Device.h"
#include "VulkanInstance.h"
#include <stdexcept>
#include <set>
#include <algorithm>
#include <stdexcept>

vk::SurfaceFormatKHR SwapChain::fetchSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	//Just looking for default.
	for (const auto& format : availableFormats) {
		if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return format;
		}
	}
	return availableFormats[0];
}

vk::PresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == vk::PresentModeKHR::eFifo) {
			return availablePresentMode;
		}
	}

	throw std::runtime_error("Couldn't find vsync support");
}

vk::Extent2D SwapChain::chooseSwapExtent(GLFWwindow* window, const vk::SurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		vk::Extent2D swapExtent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

		swapExtent.width = std::clamp(swapExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		swapExtent.height = std::clamp(swapExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return swapExtent;
	}
}

std::vector<vk::ImageView> SwapChain::createImageViews(const vk::Device& device, const std::vector<vk::Image> swapChainImages, const vk::Format format)
{
	std::vector<vk::ImageView> swapChainImageViews;
	swapChainImageViews.reserve(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		vk::ImageViewCreateInfo createInfo(
			vk::ImageViewCreateFlags(), swapChainImages[i], vk::ImageViewType::e2D, format,
			vk::ComponentMapping(vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity),
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
		);
		swapChainImageViews.push_back(device.createImageView(createInfo));
	}

	return swapChainImageViews;
}

vk::SwapchainKHR SwapChain::createSwapChain(const Device& device, const SwapChainSupportDetails& supportDetails, const vk::SurfaceFormatKHR& surfaceFormat, const vk::Extent2D& extent)
{
	const vk::PresentModeKHR presentMode = chooseSwapPresentMode(supportDetails.presentModes);

	uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;

	if (supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount) {
		imageCount = supportDetails.capabilities.maxImageCount;
	}

	const bool sameI = device.m_queueFamilies.graphicsFamily == device.m_queueFamilies.presentFamily;

	if (device.m_queueFamilies.graphicsFamily == device.m_queueFamilies.presentFamily) {
		const vk::SwapchainCreateInfoKHR createInfo(vk::SwapchainCreateFlagsKHR(), device.m_instance.m_surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
			extent, 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, nullptr, supportDetails.capabilities.currentTransform,
			vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode, VK_TRUE);

		return device.m_device.createSwapchainKHR(createInfo);
	}
	else {
		const std::vector indices = { device.m_queueFamilies.graphicsFamily.value(), device.m_queueFamilies.graphicsFamily.value() };
		const vk::SwapchainCreateInfoKHR createInfo(vk::SwapchainCreateFlagsKHR(), device.m_instance.m_surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
			extent, 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eConcurrent
			, indices, supportDetails.capabilities.currentTransform,
			vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode, VK_TRUE);

		return device.m_device.createSwapchainKHR(createInfo);
	}
}

SwapChain::SwapChain(const Device& device)
	: m_device(device),
	m_supportDetails(device.m_swapChainSupportDetails),
	m_surfaceFormat(fetchSwapSurfaceFormat(m_supportDetails.formats)),
	m_swapChainExtent(chooseSwapExtent(device.m_instance.m_window.m_window, m_supportDetails.capabilities)),
	m_swapChain(createSwapChain(device, m_supportDetails, m_surfaceFormat, m_swapChainExtent)),
	m_swapChainImages(device.m_device.getSwapchainImagesKHR(m_swapChain)),
	m_swapChainImageViews(createImageViews(device.m_device, m_swapChainImages, m_surfaceFormat.format))
{

}

SwapChain::~SwapChain()
{
	for (auto &imageView : m_swapChainImageViews) {
		vkDestroyImageView(m_device.m_device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(m_device.m_device, m_swapChain, nullptr);
}
