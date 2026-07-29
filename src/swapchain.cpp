#include "swapchain.hpp"
#include "utils.cpp"
Swapchain::Swapchain(VkDevice device, VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface, SDL_Window* window)
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

void Swapchain::create()
{
    // 1. query surface capabilities/formats/present modes
	chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface_, &surfaceCaps));
    
    // Get swapchain extent
    // For Wayland Enjoyers:
    VkExtent2D swapchainExtent{ surfaceCaps.currentExtent };
    if (surfaceCaps.currentExtent.width == 0xFFFFFFFF)
    {
        swapchainExtent = { .width = static_cast<uint32_t>(windowSize.x), .height = static_cast<uint32_t>(windowSize.y) };
    }
    swapchainCI = VkSwapchainCreateInfoKHR{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = surfaceCaps.minImageCount,
        .imageFormat = imageFormat,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent{.width = swapchainExtent.width, .height = swapchainExtent.height },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR
    };
    chk(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain));
    chk(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain));
    // 2. choose format, present mode, extent
    // 3. vkCreateSwapchainKHR
    // 4. vkGetSwapchainImagesKHR -> images_
    // 5. create a VkImageView for each image -> imageViews_
    // (no VkFramebuffer / VkRenderPass — not needed with dynamic rendering)

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
    void ChooseExtent(VkExtent2D windowExtent)
    {
		chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_,
            surface_, &surfaceCaps_));
        // Calculate the extent based on the window size and surface capabilities
        // For Wayland Enjoyers:
		extent_ = surfaceCaps_.currentExtent;
        if (surfaceCaps.currentExtent.width != 0xFFFFFFFF)
        {
            extent_ = windowExtent;
        }
        // Clamps if I want later
	}
}