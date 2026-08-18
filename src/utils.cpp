#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <vulkan/vk_enum_string_helper.h>
#include "swapchain.hpp"
// Check Functions:
static inline void chk(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		std::cerr << "Vulkan call returned an error (" << string_VkResult(result) << ")\n";
		exit(result);
	}
}


// THIS WILL LIKELY CHANGE
static inline void chkSwapchain(VkResult result, Swapchain swapchain)
{
	if (result < VK_SUCCESS)
	{
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			swapchain.needsRecreation_ = true;
			return;
		}
		std::cerr << "Vulkan call returned an error (" << string_VkResult(result) << ")\n";
		exit(result);
	}
}
static inline void chk(bool result)
{
	if (!result)
	{
		std::cerr << "Call returned an error\n";
		exit(result);
	}
}