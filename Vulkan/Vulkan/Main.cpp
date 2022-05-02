// Vulkan.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "VulkanInstance.h"
#include "Window.h"
#include "ComputePipeline.h"

int main()
{
	std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	Window window = Window(800, 600, "Test App");
	VulkanInstance vkInstance = VulkanInstance(window, "Test App", 1, 0, 0, validationLayers);
	ComputePipeline test = ComputePipeline(*vkInstance.m_device, "shaders/comp.spv");


}
