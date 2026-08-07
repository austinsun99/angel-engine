#pragma once

#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <vector>
#include "pastel_types.h"
namespace Pastel::Renderer::Vulkan {

struct VulkanDeviceQueue {
    VkQueueFamilyProperties2 properties;
    u32 queue_family_index = 0;
    u32 queue_count        = 0;
};

struct VulkanPhysicalDeviceRequirements {
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

struct VulkanPhysicalDeviceProperties {
    std::vector<VulkanDeviceQueue> device_queues;

    // UINT32_MAX indicates the queue does not exist
    u32 graphics_queue_index = UINT32_MAX;
    // UINT32_MAX indicates the queue does not exist
    u32 compute_queue_index  = UINT32_MAX;
    // UINT32_MAX indicates the queue does not exist
    u32 present_queue_index  = UINT32_MAX;
    // UINT32_MAX indicates the queue does not exist
    u32 transfer_queue_index = UINT32_MAX;

    VkPhysicalDeviceProperties2 device_properties;
    VkPhysicalDeviceMemoryProperties2 memory_properties;

    VkPhysicalDeviceFeatures2 device_features_2;
    VkPhysicalDeviceVulkan11Features device_features_11;
    VkPhysicalDeviceVulkan12Features device_features_12;
    VkPhysicalDeviceVulkan13Features device_features_13;
    VkPhysicalDeviceVulkan14Features device_features_14;

    VkSurfaceCapabilities2KHR surface_capabilities;
    std::vector<VkSurfaceFormat2KHR> surface_formats;
    std::vector<VkPresentModeKHR> surface_present_modes;
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
                                            VulkanPhysicalDeviceRequirements const &requirements);

   public:
    VulkanDevice();
    ~VulkanDevice();
    void init_device(VkInstance *vulkan_instance,
                     VkSurfaceKHR *vulkan_surface,
                     VkAllocationCallbacks *custom_allocator);
    void destroy_device();

    bool query_for_physical_device(VulkanPhysicalDeviceRequirements const &device_requirements);
    bool create_logical_device(VulkanPhysicalDeviceRequirements const &device_requirements);
    bool query_for_device_swapchain_support(VkPhysicalDevice const &device);
};

bool vulkan_get_physical_devices(VkInstance const &instance,
                                        std::vector<VkPhysicalDevice> &out_physical_devices);
bool vulkan_get_physical_device_properties(VkPhysicalDevice const &device,
                                                  VkSurfaceKHR const &surface,
                                                  VulkanPhysicalDeviceProperties &out_properties);
bool vulkan_physical_device_meets_requirements(VulkanPhysicalDeviceProperties const &properties,
                                                      VulkanPhysicalDeviceRequirements const &requirements);

}  // namespace Pastel::Renderer::Vulkan
