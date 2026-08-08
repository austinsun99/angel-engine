#pragma once

#include <vulkan/vulkan_core.h>
#include <array>
#include "core/platform/platform.h"
#include "renderer/vulkan_synchronization.hpp"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_command_buffer.h"

namespace Pastel::Renderer::Vulkan {

#define FRAMES_IN_FLIGHT 2

class VulkanRenderer {
   private:
    Platform::WindowState const &_window_state;
    VkAllocationCallbacks *_custom_allocator;

    VkInstance _instance;
    VkSurfaceKHR _surface;
    VulkanDevice _device;
    VulkanSwapchain _swapchain;

    std::vector<VulkanCommandBuffer> _graphics_command_buffers;
    std::vector<VulkanSyncObject> _sync_objects;

    u32 _current_frame = 0;
   public:
    VulkanRenderer(Platform::WindowState const &window_state) : _window_state(window_state) {
        _custom_allocator = nullptr;
    }
    ~VulkanRenderer();

    void start();
    void update_start();
    void update_end();
};

}  // namespace Pastel::Renderer::Vulkan
