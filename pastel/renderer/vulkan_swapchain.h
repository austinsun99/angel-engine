#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include "pastel_types.h"
#include "renderer/vector.hpp"
#include "renderer/vulkan_device.h"
namespace Pastel::Renderer::Vulkan {

const std::vector<Vertex> vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                      {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                      {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                      {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};
const std::vector<u32> indices     = {0, 1, 2, 2, 3, 0};

class VulkanSwapchain {
   private:
    VulkanDevice *_device                    = nullptr;
    VkAllocationCallbacks *_custom_allocator = nullptr;
    VkSurfaceKHR _surface                    = VK_NULL_HANDLE;

    VkSwapchainKHR _handle = VK_NULL_HANDLE;

    std::vector<VkImage> _images;
    std::vector<VkImageView> _image_views;

    VkSurfaceFormatKHR _selected_surface_format;
    VkExtent2D _current_extent;

    bool _initialized = false;
    bool create_image_views();

   public:
    VulkanSwapchain()  = default;
    ~VulkanSwapchain() = default;

    void init(VulkanDevice *const &device,
              VkAllocationCallbacks *const &custom_allocator,
              VkSurfaceKHR const &surface) {
        _device           = device;
        _custom_allocator = custom_allocator;
        _surface          = surface;
        _initialized      = true;
    };

    bool create_swapchain(const u32 framebuffer_width, const u32 framebuffer_height);
    bool destroy_swapchain();
    bool acquire_next_image(u64 timeout,
                            VkSemaphore const &image_available_semaphore,
                            VkFence const &fence,
                            u32 *out_index) const;

    bool create_uniform_buffers();
    bool create_descriptor_set_layout_and_pool(std::vector<VkBuffer> const &uniform_buffers);

    VkResult present(VkSemaphore const &render_complete_sem, u32 image_index);

    void selected_format(VkFormat &out_format) const {
        out_format = _selected_surface_format.format;
    }

    std::vector<VkImage> const &images() const {
        return _images;
    }

    std::vector<VkImageView> const &image_views() const {
        return _image_views;
    }
};

}  // namespace Pastel::Renderer::Vulkan
