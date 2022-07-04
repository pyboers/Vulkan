#pragma once
#include "imgui_impl_glfw.h"
#include "Device.h"
class GUIManager
{
private:
	VkCommandBuffer buffer;
public:
	GUIManager(GLFWwindow* window, VkInstance instance, Device& device, VkPhysicalDevice physicalDevice, uint32_t queueFamily, VkQueue queue,
		uint32_t minImageCount, uint32_t imageCount);


	void render() const;


		
};

