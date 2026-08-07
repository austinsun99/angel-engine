#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include "pastel_types.h"
namespace Pastel::Renderer::Vulkan {

struct VulkanDeviceQueue {
    bool active            = false;
    u32 queue_family_index = 0;
    u32 queue_count        = 0;
    u32 active_queue_count = 0;
};

struct VulkanQueryPhysicalDeviceRequirements {
    bool allow_dedicated  = true;
    bool allow_integrated = true;

    bool require_graphics = true;
    bool require_present  = true;
    bool require_transfer = true;
    bool require_compute  = true;

    bool dynamic_rendering = true;

    std::vector<const char *> required_extensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
};

class VulkanDevice {
   private:
    VkInstance *_vulkan_instance;
    VkSurfaceKHR *_vulkan_surface;
    VkAllocationCallbacks *_custom_allocator;

    VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
    VkDevice _device                  = VK_NULL_HANDLE;

    VulkanDeviceQueue _graphics_queue;
    VulkanDeviceQueue _compute_queue;
    VulkanDeviceQueue _present_queue;
    VulkanDeviceQueue _transfer_queue;

    std::vector<VkQueueFamilyProperties2> _queue_family_properties;
    VkSurfaceCapabilities2KHR _surface_capabilities;
    VkPhysicalDeviceProperties2 _physical_device_properties;
    VkPhysicalDeviceMemoryProperties2 _physical_device_memory_properties;

    VkPhysicalDeviceFeatures2 _device_features_2;
    VkPhysicalDeviceVulkan11Features _device_features_11;
    VkPhysicalDeviceVulkan12Features _device_features_12;
    VkPhysicalDeviceVulkan13Features _device_features_13;
    VkPhysicalDeviceVulkan14Features _device_features_14;

    std::vector<VkSurfaceFormat2KHR> _formats;
    std::vector<VkPresentModeKHR> _present_modes;

    bool physical_device_meets_requirements(VkPhysicalDevice const &device,
                                            VulkanQueryPhysicalDeviceRequirements const &requirements);

   public:
    VulkanDevice();
    ~VulkanDevice();
    void init_device(VkInstance *vulkan_instance,
                     VkSurfaceKHR *vulkan_surface,
                     VkAllocationCallbacks *custom_allocator);
    void destroy_device();

    bool query_for_physical_device(VulkanQueryPhysicalDeviceRequirements const &device_requirements);
    bool create_logical_device(VulkanQueryPhysicalDeviceRequirements const &device_requirements);
    bool query_for_device_swapchain_support(VkPhysicalDevice const &device);
};

}  // namespace Pastel::Renderer::Vulkan
