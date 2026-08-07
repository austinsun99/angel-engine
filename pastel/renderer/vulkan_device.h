#pragma once

#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "pastel_types.h"
#include "renderer/vulkan_instance.h"
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

#define QUEUE_INDEX_NONE UINT32_MAX
struct VulkanPhysicalDeviceProperties {
    std::vector<VulkanDeviceQueue> device_queues;
    std::unordered_set<int> queue_families_in_use;

    // UINT32_MAX indicates the queue does not exist
    u32 graphics_queue_index = QUEUE_INDEX_NONE;
    // UINT32_MAX indicates the queue does not exist
    u32 compute_queue_index  = QUEUE_INDEX_NONE;
    // UINT32_MAX indicates the queue does not exist
    u32 present_queue_index  = QUEUE_INDEX_NONE;
    // UINT32_MAX indicates the queue does not exist
    u32 transfer_queue_index = QUEUE_INDEX_NONE;

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

    std::vector<VkExtensionProperties> extension_properties;
};

class VulkanDevice {
   private:
    VkInstance _vulkan_instance;
    VkSurfaceKHR _vulkan_surface;
    VulkanPhysicalDeviceRequirements _device_requirements;
    VkAllocationCallbacks *_custom_allocator;
    VkPhysicalDevice _physical_device;
    VulkanPhysicalDeviceProperties _device_properties;

    bool setup = false;
    VkDevice _device = VK_NULL_HANDLE;
    std::unordered_map<int, int> queue_family_to_active_queue_count;

    void format_device_info_str(std::string& str) const;

   public:
    VulkanDevice() = default;
    ~VulkanDevice() = default;
    void setup_device(VkInstance const &instance,
                      VkSurfaceKHR const &surface,
                      VkAllocationCallbacks *const &custom_allocator,
                      VkPhysicalDevice const &physical_device,
                      VulkanPhysicalDeviceRequirements const &device_requirements,
                      VulkanPhysicalDeviceProperties const &device_properties);

    bool create_logical_device();
    void destroy_device();
};

bool vulkan_get_physical_devices(VkInstance const &instance, std::vector<VkPhysicalDevice> &out_physical_devices);
bool vulkan_get_physical_device_properties(VkPhysicalDevice const &device,
                                           VkSurfaceKHR const &surface,
                                           VulkanPhysicalDeviceProperties &out_properties);
bool vulkan_physical_device_meets_requirements(VulkanPhysicalDeviceProperties const &properties,
                                               VulkanPhysicalDeviceRequirements const &requirements,
                                               VulkanCreateInstanceInfo const &instance_requirements);

}  // namespace Pastel::Renderer::Vulkan
