#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "vulkan/vulkan.hpp"

class Device;
struct SwapChainSupportDetails;

class SwapChain
{
private:
	const Device& m_device;
	const SwapChainSupportDetails &m_supportDetails;
public:
	vk::SurfaceFormatKHR m_surfaceFormat;
	vk::Extent2D m_swapChainExtent;

private:
	static vk::SurfaceFormatKHR fetchSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

	static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

	static vk::Extent2D chooseSwapExtent(GLFWwindow * window, const vk::SurfaceCapabilitiesKHR& capabilities);

	static std::vector<vk::ImageView> createImageViews(const vk::Device &device, const std::vector<vk::Image> swapChainImages, const vk::Format format);

	static vk::SwapchainKHR createSwapChain(const Device &device, const SwapChainSupportDetails &supportDetails, const vk::SurfaceFormatKHR &surfaceFormat, const vk::Extent2D &extent);


public:
	vk::SwapchainKHR m_swapChain;
	std::vector<vk::Image> m_swapChainImages;

	std::vector<vk::ImageView> m_swapChainImageViews;
	SwapChain(const Device& device);

	~SwapChain();
};

