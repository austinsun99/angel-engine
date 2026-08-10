#pragma once

#include <vulkan/vulkan_core.h>
#include <array>
#include "core/platform/platform.h"
#include "renderer/vulkan_graphics_pipeline.h"
#include "renderer/vulkan_synchronization.hpp"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_command_buffer.h"

namespace Pastel::Renderer::Vulkan {

#define FRAMES_IN_FLIGHT 2

class VulkanRenderer {
   private:
    u32 _current_framebuffer_width = 0;
    u32 _current_framebuffer_height = 0;
    Platform::WindowState const &_window_state;
    VkAllocationCallbacks *_custom_allocator;

    VkInstance _instance;
    VkSurfaceKHR _surface;
    VulkanDevice _device;
    VulkanSwapchain _swapchain;
    GraphicsPipeline _graphics_pipeline;

    std::vector<VulkanCommandBuffer> _graphics_command_buffers;

    VulkanSyncObject<FRAMES_IN_FLIGHT> _sync_objects;

    u32 _current_frame = 0;
    bool _should_recreate_swapchain = false;

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
