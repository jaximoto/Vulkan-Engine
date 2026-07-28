#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <vulkan/vk_enum_string_helper.h>
// Check Functions:
static inline void chk(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		std::cerr << "Vulkan call returned an error (" << string_VkResult(result) << ")\n";
		exit(result);
	}
}


static inline void chkSwapchain(VkResult result, bool* updateSwapchain)
{
	if (result < VK_SUCCESS)
	{
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			*updateSwapchain = true;
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