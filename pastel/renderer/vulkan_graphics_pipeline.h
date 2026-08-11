#pragma once

#include <vulkan/vulkan_core.h>
#include <utility>
#include "renderer/vulkan_device.h"
#include "renderer/vulkan_swapchain.h"
namespace Pastel::Renderer::Vulkan {
class GraphicsPipeline {
   private:
    VkAllocationCallbacks *_custom_allocator;
    VulkanDevice *_device;
    VulkanSwapchain *_swapchain;

    VkPipeline _handle;
    VkPipelineLayout _layout;
    std::vector<VkShaderModule> _shader_modules;

    bool create_shader_module(const char *file_name, VkShaderModule *out_module);

   public:
    GraphicsPipeline()  = default;
    ~GraphicsPipeline() = default;
    bool init(VulkanDevice *const &device,
              VulkanSwapchain *const &swapchain,
              VkAllocationCallbacks *const &custom_allocator) {
        _device           = device;
        _swapchain        = swapchain;
        _custom_allocator = custom_allocator;
        return true;
    };

    VkPipeline const& handle() const {
        return _handle;
    }

    VkPipelineLayout const& layout() const {
        return _layout;
    }

    bool create();
    bool destroy();
};
}  // namespace Pastel::Renderer::Vulkan
