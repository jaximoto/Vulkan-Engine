#include "swapchain.hpp"
#include "utils.cpp"
Swapchain::Swapchain(VkDevice device, VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface, SDL_Window* window)
	: device_(device), physicalDevice_(physicalDevice), 
	surface_(surface), window_(window)
{
    imageFormat_ = { VK_FORMAT_B8G8R8A8_SRGB };
    create();
}

Swapchain::~Swapchain()
{
    cleanup();
}

Swapchain::Swapchain(Swapchain&& other) noexcept
    : physicalDevice_(other.physicalDevice_), device_(other.device_),
    surface_(other.surface_), window_(other.window_),
    swapchain_(other.swapchain_), images_(std::move(other.images_)),
    imageViews_(std::move(other.imageViews_)),
    imageFormat_(other.imageFormat_), extent_(other.extent_)
{
    other.swapchain_ = VK_NULL_HANDLE; // prevent double-destroy
}

Swapchain& Swapchain::operator=(Swapchain&& other) noexcept
{
    if (this != &other)
    {
        cleanup();
        physicalDevice_ = other.physicalDevice_;
        device_ = other.device_;
        surface_ = other.surface_;
        window_ = other.window_;
        swapchain_ = other.swapchain_;
        images_ = std::move(other.images_);
        imageViews_ = std::move(other.imageViews_);
        imageFormat_ = other.imageFormat_;
        extent_ = other.extent_;
        other.swapchain_ = VK_NULL_HANDLE;
    }
    return *this;
}

void Swapchain::recreate()
{
    glm::ivec2 windowSize;
    chk(SDL_GetWindowSize(window_, &windowSize.x, &windowSize.y));
    if (windowSize.x == 0 || windowSize.y == 0)
    {
        isValid_ = false;
        return; // don't touch Vulkan, don't clear updateSwapchain — try again next frame
    }

	needsRecreation_ = false;
	chk(vkDeviceWaitIdle(device_));
    create();
    // Destroy old swapchain after makiong new one
	vkDestroySwapchainKHR(device_, swapchainCI_.oldSwapchain, nullptr);
}
void Swapchain::create()
{
    // 1. query surface capabilities/formats/present modes
	glm::ivec2 windowSize;
    chk(SDL_GetWindowSize(window_, &windowSize.x, &windowSize.y));

    if (windowSize.x == 0 || windowSize.y == 0)
    {
        isValid_ = false;
        return; // don't touch Vulkan at all — nothing to create yet
    }
	VkExtent2D windowExtent{ static_cast<uint32_t>(windowSize.x), static_cast<uint32_t>(windowSize.y) };
	chooseExtent(windowExtent);
	createCI();
    // 3. vkCreateSwapchainKHR
	chk(vkCreateSwapchainKHR(device_, &swapchainCI_, nullptr, &swapchain_));
    std::cout << "Made swapchain " << std::endl;

    // Get rid of remaining image views if any
    for (auto& view : imageViews_)
    {
        if (view != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device_, view, nullptr);
        }
		
	}
    imageViews_.clear();
    // 4. vkGetSwapchainImagesKHR -> images_ which are the raw images
	chk(vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount_, nullptr));
	images_.resize(imageCount_);
    // .data() is saying give me a ptr to beginning of images array so I can write data
	chk(vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount_, images_.data()));
    // 5. create a VkImageView for each image -> imageViews_ which are the performant view of image data
	imageViews_.resize(imageCount_);
    for (auto i = 0; i < imageCount_; i++)
    {
        VkImageViewCreateInfo viewCI{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = images_[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = imageFormat_,
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
        };
        chk(vkCreateImageView(device_, &viewCI, nullptr, &imageViews_[i]));
	}
	recreateRenderCompleteSemaphores();
    isValid_ = true;
    

    
}

/*
    * Since window manages the surface, it will tell me what sizes of images that the platform
    * can accept.That is why we can't use window size directly.
    * Usually platform knows exact size so I can use that.
    * Some platforms like Wayland don't care.
    * Windows resize all the time so I need to be doing this every frame
    */
    /// <summary>
    /// Queries surface capabilities and chooses swapchain extent based on result.
    /// </summary>
    /// <param name="windowExtent">Window size converted to unint32_t</param>
void Swapchain::chooseExtent(VkExtent2D windowExtent)
{
    chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_,
        surface_, &surfaceCaps_));
    // Calculate the extent based on the window size and surface capabilities
    // For Wayland Enjoyers:
    extent_ = surfaceCaps_.currentExtent;
    if (surfaceCaps_.currentExtent.width != 0xFFFFFFFF)
    {
        extent_ = windowExtent;
    }
    // Clamps if I want later
}

void Swapchain::createCI()
{
    swapchainCI_ = VkSwapchainCreateInfoKHR{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface_,
        .minImageCount = surfaceCaps_.minImageCount,
        .imageFormat = imageFormat_,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent{.width = extent_.width, .height = extent_.height },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .oldSwapchain = swapchain_ // Okay bc this will be null handle on first run
    };
	
}

void Swapchain::recreateRenderCompleteSemaphores()
{
    // first destroy any remaining render complete semaphores:
    for (auto& semaphore : renderCompleteSemaphores_)
    {
		vkDestroySemaphore(device_, semaphore, nullptr);
    }
	renderCompleteSemaphores_.resize(imageCount_);
    // then make new ones:
    for (auto& semaphore : renderCompleteSemaphores_)
    {
        chk(vkCreateSemaphore(device_, &semaphoreCI, nullptr, &semaphore));
	}
}

