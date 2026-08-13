#include <vulkan/vulkan_core.h>
#include <algorithm>
#include <vector>
#include "vulkan_swapchain.h"
#include "core/logging/logger.h"
#include "core/logging/asserts.h"
#include "vulkan_utils.h"

namespace Pastel::Renderer::Vulkan {
bool VulkanSwapchain::create_swapchain(const u32 framebuffer_width, const u32 framebuffer_height) {
    if (!_initialized) {
        CORE_LOG_ERROR("(Vulkan-Swapchain) Swapchain has not been initialized. Initialize the swapchain first.")
        return false;
    }

    VulkanPhysicalDeviceProperties const &dev_properties    = _device->device_properties();
    std::vector<VkSurfaceFormat2KHR> const &surface_formats = dev_properties.surface_formats;

    PASTEL_ASSERT(surface_formats.size() > 0);

    // Query for format and colourspace
    _selected_surface_format = surface_formats[0].surfaceFormat;
    for (VkSurfaceFormat2KHR const &fmt : surface_formats) {
        // @todo: allow customization of wanted format/colourspace
        if (fmt.surfaceFormat.format == VK_FORMAT_R8G8B8A8_SRGB &&
            fmt.surfaceFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
            _selected_surface_format = fmt.surfaceFormat;
            break;
        }
    }

    // Query for present mode
    VkPresentModeKHR selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (VkPresentModeKHR const &present : dev_properties.surface_present_modes) {
        if (present == VK_PRESENT_MODE_MAILBOX_KHR) {
            selected_present_mode = present;
            break;
        }
    }

    // Query for extents
    VkSurfaceCapabilitiesKHR const &surface_capabilities = dev_properties.surface_capabilities.surfaceCapabilities;
    _current_extent                                      = surface_capabilities.currentExtent;
    if (surface_capabilities.currentExtent.width == std::numeric_limits<u32>::max()) {
        _current_extent = {.width  = std::clamp(framebuffer_width,
                                                surface_capabilities.minImageExtent.width,
                                                surface_capabilities.maxImageExtent.width),
                           .height = std::clamp(framebuffer_height,
                                                surface_capabilities.minImageExtent.height,
                                                surface_capabilities.maxImageExtent.height)};
    }

    // Query and set minimum image count
    u32 swapchain_image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount != 0 && swapchain_image_count > surface_capabilities.maxImageCount) {
        swapchain_image_count = surface_capabilities.maxImageCount;
    }

    // Set sharing mode based on graphics and present queue family index
    VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
    u32 index_count            = 1;
    std::vector<u32> family_indices{
        dev_properties.graphics_queue_index,
    };

    if (dev_properties.present_queue_index != dev_properties.graphics_queue_index) {
        sharing_mode = VK_SHARING_MODE_CONCURRENT;
        index_count  = 2;
        family_indices.push_back(dev_properties.present_queue_index);
    }

    // Create the swapchain
    const VkSwapchainCreateInfoKHR swapchain_create_info{
        .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext                 = nullptr,
        .flags                 = 0,
        .surface               = _surface,
        .minImageCount         = swapchain_image_count,
        .imageFormat           = _selected_surface_format.format,
        .imageColorSpace       = _selected_surface_format.colorSpace,
        .imageExtent           = _current_extent,
        .imageArrayLayers      = 1,
        .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode      = sharing_mode,
        .queueFamilyIndexCount = index_count,
        .pQueueFamilyIndices   = &family_indices[0],
        .preTransform          = surface_capabilities.currentTransform,
        .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode           = selected_present_mode,
        .clipped               = VK_TRUE,
        .oldSwapchain          = VK_NULL_HANDLE,
    };

    VK_CHECK_RESULT(vkCreateSwapchainKHR(_device->device(), &swapchain_create_info, _custom_allocator, &_handle));

    u32 image_count = 0;
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(_device->device(), _handle, &image_count, nullptr));
    PASTEL_ASSERT(swapchain_image_count == image_count)
    _images.resize(image_count);
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(_device->device(), _handle, &image_count, &_images[0]));

    if (!create_image_views()) {
        return false;
    }

    return true;
}

VkResult VulkanSwapchain::present(VkSemaphore const &render_complete_sem, u32 image_index) {
    const VkPresentInfoKHR present_info{
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext              = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &render_complete_sem,
        .swapchainCount     = 1,
        .pSwapchains        = &_handle,
        .pImageIndices      = &image_index,
        .pResults           = nullptr,
    };

    return vkQueuePresentKHR(_device->present_queue(), &present_info);
}

bool VulkanSwapchain::create_image_views() {
    PASTEL_ASSERT(_images.size() > 0);

    _image_views.resize(_images.size());
    VkImageViewCreateInfo view_create_info{
        .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = 0,
        .image    = nullptr,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format   = _selected_surface_format.format,
        .components =
            {
                .r = VK_COMPONENT_SWIZZLE_R,
                .g = VK_COMPONENT_SWIZZLE_G,
                .b = VK_COMPONENT_SWIZZLE_B,
                .a = VK_COMPONENT_SWIZZLE_A,
            },
        .subresourceRange =
            {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                // mip level count must be 1 for swapchain images
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1,
            },
    };

    for (u64 i = 0; i < _images.size(); ++i) {
        view_create_info.image = _images[i];
        VK_CHECK_RESULT(vkCreateImageView(_device->device(), &view_create_info, _custom_allocator, &_image_views[i]));
    }

    return true;
}

bool VulkanSwapchain::destroy_swapchain() {
    for (VkImageView const &view : _image_views) {
        vkDestroyImageView(_device->device(), view, _custom_allocator);
        _image_views.clear();
    }
    if (_handle != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(_device->device(), _handle, _custom_allocator);
        _handle = VK_NULL_HANDLE;
    }
    return true;
}

bool VulkanSwapchain::acquire_next_image(u64 timeout,
                                         VkSemaphore const &image_available_semaphore,
                                         VkFence const &fence,
                                         u32 *out_index) const {
    VkAcquireNextImageInfoKHR info{
        .sType      = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
        .pNext      = nullptr,
        .swapchain  = _handle,
        .timeout    = timeout,
        .semaphore  = image_available_semaphore,
        .fence      = fence,
        .deviceMask = 1,
    };

    VK_CHECK_RESULT(vkAcquireNextImage2KHR(_device->device(), &info, out_index));
    return true;
}

}  // namespace Pastel::Renderer::Vulkan
