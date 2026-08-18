#pragma once
#include <vulkan/vulkan.h>
#include <volk/volk.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vector>
#include <array>
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

	bool queryWindow();
	void getImageIndex(uint32_t frameIndex);
	void recreate();
	void cleanup();
	void resizeWindowEvent(SDL_Event event);
	VkSwapchainKHR handle() const { return swapchain_; }
	VkFormat imageFormat() const { return imageFormat_; }
	VkExtent2D extent() const { return extent_; }
	const std::vector<VkImage>& images() const { return images_; }
	const std::vector<VkImageView>& imageViews() const { return imageViews_; }
	uint32_t imageCount() const { return static_cast<uint32_t>(images_.size()); }
	uint32_t frameIndex_ = { 0 };
	uint32_t imageIndex_ = { 0 };
	bool needsRecreation_ = false;
	VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
	std::vector<VkSemaphore> renderCompleteSemaphores_;
	std::array<VkSemaphore, 2> imageAcquiredSemaphores_;

private:
	void create();
	void chooseExtent(VkExtent2D windowExtent);
	void createCI();
	bool isValid() const { return isValid_; }
	void recreateRenderCompleteSemaphores();
	void createImageAcquiredSemaphores();
	

	// Borrowed — owned elsewhere (Instance/Device/Window classes)
	VkDevice device_;
	VkPhysicalDevice physicalDevice_;
	VkSurfaceKHR surface_;
	SDL_Window* window_;

	// Owned
	
	VkSwapchainCreateInfoKHR swapchainCI_{};
	std::vector<VkImage> images_;
	std::vector<VkImageView> imageViews_;
	
	uint32_t imageCount_ = { 0 };
	
	VkSurfaceCapabilitiesKHR surfaceCaps_{};
	VkFormat imageFormat_;
	glm::ivec2 windowSize_{};
	VkExtent2D extent_;
	VkSemaphoreCreateInfo semaphoreCI{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};
	bool isValid_ = false;
	uint32_t maxFramesInFlight_ = 2;
};