#pragma once
#include <vulkan/vulkan.h>
#include <volk/volk.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vector>
#include <iostream>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class Swapchain
{
public:
	Swapchain(VkDevice device, VkPhysicalDevice physicalDevice,
			VkSurfaceKHR surface, SDL_Window* window);
	~Swapchain();

	// Move-only: owns Vulkan handles, copying would double-destroy
	Swapchain(const Swapchain&) = delete;
	Swapchain& operator=(const Swapchain&) = delete;
	Swapchain(Swapchain&& other) noexcept;
	Swapchain& operator=(Swapchain&& other) noexcept;

	void recreate();
	VkSwapchainKHR handle() const { return swapchain_; }
	VkFormat imageFormat() const { return imageFormat_; }
	VkExtent2D extent() const { return extent_; }
	const std::vector<VkImage>& images() const { return images_; }
	const std::vector<VkImageView>& imageViews() const { return imageViews_; }
	uint32_t imageCount() const { return static_cast<uint32_t>(images_.size()); }


private:
	void create();
	void cleanup();

	// Borrowed — owned elsewhere (Instance/Device/Window classes)
	VkDevice device_;
	VkPhysicalDevice physicalDevice_;
	VkSurfaceKHR surface_;
	SDL_Window* window_;

	// Owned
	VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
	std::vector<VkImage> images_;
	std::vector<VkImageView> imageViews_;
	VkSurfaceCapabilitiesKHR surfaceCaps_{};
	VkFormat imageFormat_;
	VkExtent2D extent_;
};