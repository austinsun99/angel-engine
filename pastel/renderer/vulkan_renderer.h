#pragma once

#include <vulkan/vulkan_core.h>
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
    u32 _current_framebuffer_width  = 0;
    u32 _current_framebuffer_height = 0;
    const Platform::Window* _window_state;
    VkAllocationCallbacks *_custom_allocator;

    VkInstance _instance;
    VkSurfaceKHR _surface;
    VulkanDevice _device;
    VulkanSwapchain _swapchain;
    GraphicsPipeline _graphics_pipeline;

    VkBuffer _vertex_buffer = VK_NULL_HANDLE;
    VkDeviceMemory _vertex_buffer_memory;
    VkBuffer _index_buffer = VK_NULL_HANDLE;
    VkDeviceMemory _index_buffer_memory;
    std::vector<VkBuffer> _uniform_buffers;
    std::vector<VkDeviceMemory> _uniform_buffers_memory;
    std::vector<void *> _uniform_buffers_map;

    VkDescriptorSetLayout _descriptor_set_layout;
    VkDescriptorPool _descriptor_pool;
    std::vector<VkDescriptorSet> _descriptor_sets;

    std::vector<VulkanCommandBuffer> _graphics_command_buffers;

    VulkanSyncObject<FRAMES_IN_FLIGHT> _sync_objects;

    u32 _current_frame              = 0;
    bool _should_recreate_swapchain = false;

    void update_uniform_buffer(u32 current_image);

   public:
    VulkanRenderer() = default;
    ~VulkanRenderer();

    void init(Platform::Window const& window_state, VkAllocationCallbacks* custom_allocator);
    void update_start();
    void update_end();
};

}  // namespace Pastel::Renderer::Vulkan
