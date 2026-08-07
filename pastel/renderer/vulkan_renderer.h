#pragma once

#include <vulkan/vulkan_core.h>
#include "core/platform/platform.h"
namespace Pastel::Renderer::Vulkan {

class VulkanRenderer {
   private:
    Platform::WindowState const &_window_state;

    VkInstance _vulkan_instance;
   public:
    VulkanRenderer(Platform::WindowState const &window_state);
    ~VulkanRenderer();
    void start();
};

}  // namespace Pastel::Renderer::Vulkan
