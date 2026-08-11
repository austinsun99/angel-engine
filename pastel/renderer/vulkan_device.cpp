#include "vulkan_device.h"

#include <vulkan/vulkan_core.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <format>
#include <unordered_set>
#include <vector>
#include "core/logging/asserts.h"
#include "core/logging/logger.h"
#include "renderer/vulkan_instance.h"
#include "renderer/vulkan_utils.h"

namespace Pastel::Renderer::Vulkan {

void VulkanDevice::setup_device(VkInstance const &instance,
                                VkSurfaceKHR const &surface,
                                VkAllocationCallbacks *const &custom_allocator,
                                VkPhysicalDevice const &physical_device,
                                VulkanPhysicalDeviceRequirements const &device_requirements,
                                VulkanPhysicalDeviceProperties const &device_properties) {
    _vulkan_instance     = instance;
    _vulkan_surface      = surface;
    _device_requirements = device_requirements;
    _custom_allocator    = custom_allocator;
    _physical_device     = physical_device;
    _device_properties   = device_properties;
    setup                = true;

    std::string device_info_str;
    format_device_info_str(device_info_str);
    CORE_LOG_INFO(device_info_str.c_str());
}

bool VulkanDevice::create_logical_device() {
    if (!setup) {
        CORE_LOG_ERROR("(Vulkan-Device) Must setup device first")
        return false;
    }
    CORE_LOG_INFO("(Vulkan-Device) Creating logical device...")
    CORE_LOG_INFO("(Vulkan-Device) Creating device queues...")

    const std::vector<float> queue_priorities{0.5f, 1.0f};

    std::unordered_set<u32> queue_family_indices;
    std::vector<VkDeviceQueueCreateInfo> queue_create_info;
    queue_create_info.clear();

    if (_device_requirements.require_graphics) queue_family_indices.insert(_device_properties.graphics_queue_index);
    if (_device_requirements.require_transfer) queue_family_indices.insert(_device_properties.transfer_queue_index);
    if (_device_requirements.require_compute) queue_family_indices.insert(_device_properties.compute_queue_index);
    if (_device_requirements.require_present) queue_family_indices.insert(_device_properties.present_queue_index);

    for (u32 const &index : queue_family_indices) {
        // const u32 max_queue_creation_count =
        //     _device_properties.device_queues[index].properties.queueFamilyProperties.queueCount;
        const u32 actual_queue_creation_count = 1;  // @todo: customize? If we do customize, make sure to set the queue
                                                    // indices below in the get device queues
        PASTEL_ASSERT(queue_priorities.size() >= actual_queue_creation_count);

        VkDeviceQueueCreateInfo queue_info = {
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext            = nullptr,
            .flags            = 0,
            .queueFamilyIndex = index,
            .queueCount       = actual_queue_creation_count,
            .pQueuePriorities = &queue_priorities[0],
        };
        queue_create_info.push_back(queue_info);
        CORE_LOG_DEBUG("(Vulkan-Device) Creating %d queues for queue family %d", actual_queue_creation_count, index);
    }

    VkPhysicalDeviceFeatures2 device_features_2{};
    VkPhysicalDeviceVulkan11Features device_features_11{};
    VkPhysicalDeviceVulkan12Features device_features_12{};
    VkPhysicalDeviceVulkan13Features device_features_13{};
    VkPhysicalDeviceVulkan14Features device_features_14{};
    device_features_14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
    device_features_13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    device_features_12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    device_features_11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    device_features_2.sType  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    device_features_14.pNext = nullptr;
    device_features_13.pNext = &device_features_14;
    device_features_12.pNext = &device_features_13;
    device_features_11.pNext = &device_features_12;
    device_features_2.pNext  = &device_features_11;

    device_features_13.synchronization2 = VK_TRUE;

    if (_device_requirements.dynamic_rendering) {
        PASTEL_ASSERT(_device_properties.device_features_13.dynamicRendering == VK_TRUE);
        device_features_13.dynamicRendering = VK_TRUE;
        CORE_LOG_DEBUG("(Vulkan-Device) Enabling dynamic rendering");
    }

    VkDeviceCreateInfo device_create_info{
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = &device_features_2,
        .flags                   = 0,
        .queueCreateInfoCount    = static_cast<uint32_t>(queue_create_info.size()),
        .pQueueCreateInfos       = &queue_create_info[0],
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = nullptr,
        .enabledExtensionCount   = static_cast<uint32_t>(_device_requirements.required_extensions.size()),
        .ppEnabledExtensionNames = &_device_requirements.required_extensions[0],
        .pEnabledFeatures        = nullptr,
    };
    vkCreateDevice(_physical_device, &device_create_info, _custom_allocator, &_device);
    CORE_LOG_INFO("(Vulkan-Device) Successfully created logical device.")

    VkDeviceQueueInfo2 queue_info = {
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
        .pNext            = nullptr,
        .flags            = 0,
        .queueFamilyIndex = 0,
        .queueIndex       = 0,  // Set to zero since we specified the max number of queues to be 1 above
    };

    // Obtain the device queues
    _queues.resize(_device_properties.device_queues.size());
    for (u32 const &index : queue_family_indices) {
        queue_info.queueFamilyIndex = index;
        vkGetDeviceQueue2(_device, &queue_info, &_queues[index]);
    }

    return true;
}  // namespace Pastel::Renderer::Vulkan

void VulkanDevice::destroy_device() {
    vkDestroyDevice(_device, _custom_allocator);
    _device = VK_NULL_HANDLE;
}

void VulkanDevice::destroy_graphics_command_pool() {
    vkDestroyCommandPool(_device, _graphics_command_pool, _custom_allocator);
    _graphics_command_pool = VK_NULL_HANDLE;
}

bool vulkan_get_physical_devices(VkInstance const &instance, std::vector<VkPhysicalDevice> &out_physical_devices) {
    u32 physical_device_count;
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr));
    out_physical_devices.resize(physical_device_count);
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &physical_device_count, &out_physical_devices[0]));
    return true;
}

bool vulkan_get_physical_device_properties(VkPhysicalDevice const &device,
                                           VkSurfaceKHR const &surface,
                                           VulkanPhysicalDeviceProperties &out_properties) {
    // Query device properties, memory properties, and features
    out_properties.device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    out_properties.device_properties.pNext = nullptr;
    vkGetPhysicalDeviceProperties2(device, &out_properties.device_properties);

    out_properties.memory_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    out_properties.memory_properties.pNext = nullptr;
    vkGetPhysicalDeviceMemoryProperties2(device, &out_properties.memory_properties);

    out_properties.device_features_14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
    out_properties.device_features_13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    out_properties.device_features_12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    out_properties.device_features_11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    out_properties.device_features_2.sType  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    out_properties.device_features_14.pNext = nullptr;
    out_properties.device_features_13.pNext = &out_properties.device_features_14;
    out_properties.device_features_12.pNext = &out_properties.device_features_13;
    out_properties.device_features_11.pNext = &out_properties.device_features_12;
    out_properties.device_features_2.pNext  = &out_properties.device_features_11;
    vkGetPhysicalDeviceFeatures2(device, &out_properties.device_features_2);

    vulkan_query_swapchain_info(device, surface, out_properties);

    // Query queue information
    u32 queue_family_properties_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(device, &queue_family_properties_count, nullptr);

    std::vector<VkQueueFamilyProperties2> queue_family_properties;
    queue_family_properties.assign(queue_family_properties_count,
                                   VkQueueFamilyProperties2{
                                       .sType                 = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2,
                                       .pNext                 = nullptr,
                                       .queueFamilyProperties = {},
                                   });

    // the index of the out vector directly corresponds to the queue family index
    vkGetPhysicalDeviceQueueFamilyProperties2(device, &queue_family_properties_count, &queue_family_properties[0]);
    out_properties.device_queues.resize(queue_family_properties_count);
    for (u64 i = 0; i < queue_family_properties_count; ++i) {
        out_properties.device_queues[i].properties = queue_family_properties[i];
    }

    // pick the necessary queues.
    // We set the queue indices as the largest queue family index that supports the feature. All queues selection
    // follows this rule, with the exception of the present queue in which we first try to assign it to the same queue
    // family index as the graphics queue. The present queue follows the aforementioned rule iff the graphics queue does
    // not have present.
    for (u64 i = 0; i < out_properties.device_queues.size(); ++i) {
        // @hack: since we set queue index none to be UINT32_MAX, we check that the index is not actaully UINT32_MAX.
        // This should quite litearlly never happen.
        PASTEL_ASSERT(i != QUEUE_INDEX_NONE)

        int const &queue_family_index           = i;
        VkQueueFamilyProperties const &property = out_properties.device_queues[i].properties.queueFamilyProperties;

        VkBool32 present_support;
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(device, queue_family_index, surface, &present_support));

        // @: consider placing this information into the vulkan queue
        bool has_graphics = (property.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
        bool has_transfer = (property.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0;
        bool has_compute  = (property.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
        bool has_present  = vkb_to_b(present_support);

        if (has_graphics) {
            out_properties.graphics_queue_index = queue_family_index;

            // Attempt to set the present queue as the same as the graphics queue
            if (has_present) {
                out_properties.present_queue_index = queue_family_index;
            }
        }
        if (has_transfer) out_properties.transfer_queue_index = queue_family_index;
        if (has_compute) out_properties.compute_queue_index = queue_family_index;

        // We set the present queue as separate from the graphics queue iff the present queue exists and has not been
        // set yet.
        if (has_present && out_properties.present_queue_index != QUEUE_INDEX_NONE)
            out_properties.present_queue_index = queue_family_index;
    }

    // if (out_properties.graphics_queue_index != QUEUE_INDEX_NONE)
    //     out_properties.queue_families_in_use.insert(out_properties.graphics_queue_index);
    //
    // if (out_properties.transfer_queue_index != QUEUE_INDEX_NONE)
    //     out_properties.queue_families_in_use.insert(out_properties.transfer_queue_index);
    //
    // if (out_properties.present_queue_index != QUEUE_INDEX_NONE)
    //     out_properties.queue_families_in_use.insert(out_properties.present_queue_index);
    //
    // if (out_properties.compute_queue_index != QUEUE_INDEX_NONE)
    //     out_properties.queue_families_in_use.insert(out_properties.compute_queue_index);

    // Query for device extensions
    u32 device_extension_count = 0;
    VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &device_extension_count, nullptr));
    out_properties.extension_properties.resize(device_extension_count);
    VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(device,
                                                         nullptr,
                                                         &device_extension_count,
                                                         &out_properties.extension_properties[0]));
    return true;
}

bool vulkan_physical_device_meets_requirements(VulkanPhysicalDeviceProperties const &properties,
                                               VulkanPhysicalDeviceRequirements const &requirements,
                                               VulkanCreateInstanceInfo const &instance_requirements) {
    u32 const &api_version = properties.device_properties.properties.apiVersion;
    bool device_meets_api_version_requirements =
        VK_API_VERSION_MAJOR(api_version) >= instance_requirements.min_version_major &&
        VK_API_VERSION_MINOR(api_version) >= instance_requirements.min_version_minor &&
        VK_API_VERSION_PATCH(api_version) >= instance_requirements.min_version_patch;

    bool device_type_requirement_met        = false;
    VkPhysicalDeviceType const &device_type = properties.device_properties.properties.deviceType;
    if (requirements.allow_dedicated && device_type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        device_type_requirement_met = true;
    if (requirements.allow_integrated && device_type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        device_type_requirement_met = true;
    if (device_type_requirement_met) CORE_LOG_DEBUG("(Vulkan-Device) Device meets device type requirements.")

    bool device_meets_feature_requirements = true;
    if (requirements.dynamic_rendering && vkb_to_b(!properties.device_features_13.dynamicRendering)) {
        CORE_LOG_DEBUG("(Vulkan-Device) Dynamic rendering is required but is not found on device.")
        device_meets_feature_requirements = false;
    }
    if (device_meets_feature_requirements) CORE_LOG_DEBUG("(Vulkan-Device) Device meets feature requirements.")

    bool device_meets_queue_requirements =
        !(requirements.require_graphics && properties.graphics_queue_index == QUEUE_INDEX_NONE) &&
        !(requirements.require_compute && properties.compute_queue_index == QUEUE_INDEX_NONE) &&
        !(requirements.require_transfer && properties.transfer_queue_index == QUEUE_INDEX_NONE) &&
        !(requirements.require_present && properties.present_queue_index == QUEUE_INDEX_NONE);
    if (device_meets_queue_requirements) CORE_LOG_DEBUG("(Vulkan-Device) Device meets queue family requirements.")

    bool has_swapchain_support = properties.surface_present_modes.size() > 0 && properties.surface_formats.size() > 0;
    if (has_swapchain_support) CORE_LOG_DEBUG("(Vulkan-Device) Device meets swapchain requirements.")

    bool has_required_extensions = true;
    for (const char *const &require : requirements.required_extensions) {
        CORE_LOG_DEBUG("(Vulkan-Device) Checking if device has required extension %s", require)
        bool found = false;
        for (VkExtensionProperties const &extension : properties.extension_properties) {
            if (std::strcmp(extension.extensionName, require) == 0) {
                found = true;
                CORE_LOG_DEBUG("(Vulkan-Device) Extension %s found", require)
            }
        }
        if (!found) {
            has_required_extensions = false;
            break;
        }
    }

    return device_meets_api_version_requirements && device_type_requirement_met && device_meets_feature_requirements &&
           device_meets_queue_requirements && has_swapchain_support && has_required_extensions;
}

bool VulkanDevice::create_graphics_command_pool(VkCommandPoolCreateFlags create_flags) {
    VkCommandPoolCreateInfo create_info{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = create_flags,
        .queueFamilyIndex = _device_properties.graphics_queue_index,
    };

    VK_CHECK_RESULT(vkCreateCommandPool(_device, &create_info, _custom_allocator, &_graphics_command_pool));
    return true;
}

void VulkanDevice::format_device_info_str(std::string &str) const {
    u32 const &api_version = _device_properties.device_properties.properties.apiVersion;
    u32 const ver_major    = VK_API_VERSION_MAJOR(api_version);
    u32 const ver_minor    = VK_API_VERSION_MINOR(api_version);
    u32 const ver_patch    = VK_API_VERSION_PATCH(api_version);

    str.append(std::format("\n[Vulkan Device Info]\n for ({})\n\n",
                           _device_properties.device_properties.properties.deviceName));
    str.append(std::format("API Version (major.minor.patch) {}.{}.{}\n", ver_major, ver_minor, ver_patch));
    str.append("Queue Family Info (index):\n");
    if (_device_requirements.require_graphics)
        str.append(std::format("Graphics: {}\n", _device_properties.graphics_queue_index));
    if (_device_requirements.require_present)
        str.append(std::format("Present: {}\n", _device_properties.present_queue_index));
    if (_device_requirements.require_transfer)
        str.append(std::format("Transfer: {}\n", _device_properties.transfer_queue_index));
    if (_device_requirements.require_compute)
        str.append(std::format("Compute: {}\n", _device_properties.compute_queue_index));

    str.append("\nDevice Memory Properties:\n");
    VkPhysicalDeviceMemoryProperties const &memory_properties = _device_properties.memory_properties.memoryProperties;
    for (u64 i = 0; i < memory_properties.memoryHeapCount; ++i) {
        str.append(
            std::format("Heap {}: {} MB\n",
                        i,
                        _device_properties.memory_properties.memoryProperties.memoryHeaps[i].size / 1024 / 1024));
    }
    str.append("\n");
}

bool vulkan_query_swapchain_info(VkPhysicalDevice const &device,
                                 VkSurfaceKHR const &surface,
                                 VulkanPhysicalDeviceProperties &out_properties) {
    // Surface information for swapchain
    VkPhysicalDeviceSurfaceInfo2KHR surface_info{
        .sType   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
        .pNext   = nullptr,
        .surface = surface,
    };

    out_properties.surface_capabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
    out_properties.surface_capabilities.pNext = nullptr;
    VK_CHECK_RESULT(
        vkGetPhysicalDeviceSurfaceCapabilities2KHR(device, &surface_info, &out_properties.surface_capabilities));

    u32 surface_format_count = 0;
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surface_info, &surface_format_count, nullptr));
    out_properties.surface_formats.assign(surface_format_count,
                                          VkSurfaceFormat2KHR{
                                              .sType         = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR,
                                              .pNext         = nullptr,
                                              .surfaceFormat = {},
                                          });
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormats2KHR(device,
                                                          &surface_info,
                                                          &surface_format_count,
                                                          &out_properties.surface_formats[0]));

    u32 surface_present_mode_count = 0;
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &surface_present_mode_count, nullptr));
    out_properties.surface_present_modes.resize(surface_present_mode_count);
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device,
                                                              surface,
                                                              &surface_present_mode_count,
                                                              &out_properties.surface_present_modes[0]));

    return true;
}

bool VulkanDevice::find_suitable_memory_type(u32 type_filter, u32 property_flags, u32 *out_index) {
    VkPhysicalDeviceMemoryProperties const &memory_properties = _device_properties.memory_properties.memoryProperties;
    for (u64 i = 0; i < memory_properties.memoryTypeCount; ++i) {
        if ((type_filter & (1 << i)) &&
            (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
            *out_index = i;
            return true;
        }
    }
    return false;
}

}  // namespace Pastel::Renderer::Vulkan
