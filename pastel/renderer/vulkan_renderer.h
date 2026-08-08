#pragma once

#include <vulkan/vulkan_core.h>
#include <optional>
#include "core/platform/platform.h"
#include "renderer/vulkan_device.h"
#include "renderer/vulkan_swapchain.h"
namespace Pastel::Renderer::Vulkan {

class VulkanRenderer {
   private:
    Platform::WindowState const &_window_state;
    VkAllocationCallbacks *_custom_allocator;

    VkInstance _instance;
    VkSurfaceKHR _surface;
    VulkanDevice _device;
    VulkanSwapchain _swapchain;

   public:
    VulkanRenderer(Platform::WindowState const &window_state) : _window_state(window_state) {
        _custom_allocator = nullptr;
    }
    ~VulkanRenderer();
    void start();
};

}  // namespace Pastel::Renderer::Vulkan
