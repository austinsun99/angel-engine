#pragma once

#include <vulkan/vulkan_core.h>
#include "core/platform/platform.h"
#include "renderer/vulkan_device.h"
namespace Pastel::Renderer::Vulkan {

class VulkanRenderer {
   private:
    Platform::WindowState const &_window_state;
    VkAllocationCallbacks *_custom_allocator;

    VkInstance _vulkan_instance;
    VkSurfaceKHR _vulkan_surface;

   public:
    VulkanRenderer(Platform::WindowState const &window_state);
    ~VulkanRenderer();
    void start();
};

}  // namespace Pastel::Renderer::Vulkan
