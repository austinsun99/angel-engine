#include "vulkan_device.h"

#include <vulkan/vulkan_core.h>
#include <cstring>
#include <string>
#include <unordered_set>
#include <vector>
#include <format>
#include "core/logging/asserts.h"
#include "core/logging/logger.h"
#include "renderer/vulkan_utils.h"
namespace Pastel::Renderer::Vulkan {

static VkDeviceQueueCreateInfo get_queue_create_info(VulkanDeviceQueue const &queue);

VulkanDevice::VulkanDevice() {
}

VulkanDevice::~VulkanDevice() {
}

void VulkanDevice::init_device(VkInstance *vulkan_instance,
                               VkSurfaceKHR *vulkan_surface,
                               VkAllocationCallbacks *custom_allocator) {
    _vulkan_instance  = vulkan_instance;
    _vulkan_surface   = vulkan_surface;
    _custom_allocator = custom_allocator;
}

void VulkanDevice::destroy_device() {
    vkDestroyDevice(_device, _custom_allocator);
}

bool VulkanDevice::query_for_physical_device(VulkanQueryPhysicalDeviceRequirements const &device_requirements) {
    (void)device_requirements;

    u32 physical_device_count;
    std::vector<VkPhysicalDevice> physical_devices;
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(*_vulkan_instance, &physical_device_count, nullptr));

    physical_devices.resize(physical_device_count);
    if (physical_device_count == 0) {
        CORE_LOG_FATAL("(Vulkan-device) Could not find any devices for vulkan.")
        return false;
    }

    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(*_vulkan_instance, &physical_device_count, &physical_devices[0]));

    bool device_found = false;
    CORE_LOG_INFO("(Vulkan-device) Enumerating (%d) and choosing a physical device", physical_device_count)
    for (u64 i = 0; i < physical_device_count; ++i) {
        if (physical_device_meets_requirements(physical_devices[i], device_requirements)) {
            device_found     = true;
            _physical_device = physical_devices[i];
            break;
        }
    }
    if (device_found) {
        CORE_LOG_INFO("(Vulkan-device) Physical device found");
        PASTEL_ASSERT(_physical_device != VK_NULL_HANDLE)
    } else {
        CORE_LOG_ERROR("(Vulkan-device) Failed to find physical device with requirements")
        return false;
    }

    // @todo: consider separating filling out device details, then checking whether the device meets requirements.

    // Query memory properties
    _physical_device_memory_properties = {
        .sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
        .pNext            = nullptr,
        .memoryProperties = {},
    };
    vkGetPhysicalDeviceMemoryProperties2(_physical_device, &_physical_device_memory_properties);

    // Property device features
    _device_features_14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
    _device_features_14.pNext = nullptr;

    _device_features_13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    _device_features_13.pNext = &_device_features_14;

    _device_features_12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    _device_features_12.pNext = &_device_features_13;

    _device_features_11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    _device_features_11.pNext = &_device_features_12;

    _device_features_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    _device_features_2.pNext = &_device_features_11;
    vkGetPhysicalDeviceFeatures2(_physical_device, &_device_features_2);

    if (device_requirements.dynamic_rendering && !_device_features_13.dynamicRendering) {
        CORE_LOG_ERROR("(Vulkan-device) Dynamic Rendering not supported");
        return false;
    } else if (device_requirements.dynamic_rendering) {
        CORE_LOG_DEBUG("Dynamic Rendering supported");
    }

    CORE_LOG_INFO("(Vulkan) Found device that meets requirement. Displaying device info")

    auto const &properties = _physical_device_properties.properties;
    // u64 api_version_msg_size;
    // const char *api_version = printf_alloc_vk_api_version(properties.apiVersion, &api_version_msg_size);
    //
    // u64 driver_version_msg_size;
    // const char *driver_version = printf_alloc_vk_api_version(properties.driverVersion, &driver_version_msg_size);
    //
    // const char *device_type_str = physical_device_type_to_str(properties.deviceType);

    std::string device_info_str = std::string();

    device_info_str.append(std::format("\n\nDevice ({}):", properties.deviceName));
    device_info_str.append("\nQueue Family Info:\n");
    if (_graphics_queue.active)
        device_info_str.append(std::format("Graphics Queue Index: {}\n", _graphics_queue.queue_family_index));
    if (_transfer_queue.active)
        device_info_str.append(std::format("Transfer Queue Index: {}\n", _transfer_queue.queue_family_index));
    if (_present_queue.active)
        device_info_str.append(std::format("Present Queue Index: {}\n", _present_queue.queue_family_index));
    if (_compute_queue.active)
        device_info_str.append(std::format("Compute Queue Index: {}\n", _compute_queue.queue_family_index));
    CORE_LOG_INFO(device_info_str.c_str());

    return true;
}

bool VulkanDevice::create_logical_device(VulkanQueryPhysicalDeviceRequirements const &device_requirements) {
    std::vector<VkDeviceQueueCreateInfo> queue_create_info;
    std::unordered_set<int> unique_queue_family_indices;
    if (device_requirements.require_graphics) {
        queue_create_info.push_back(get_queue_create_info(_graphics_queue));
        unique_queue_family_indices.insert(_graphics_queue.queue_family_index);
    }
    if (device_requirements.require_transfer &&
        !unique_queue_family_indices.contains(_transfer_queue.queue_family_index)) {
        queue_create_info.push_back(get_queue_create_info(_transfer_queue));
        unique_queue_family_indices.insert(_transfer_queue.queue_family_index);
    }
    if (device_requirements.require_present &&
        !unique_queue_family_indices.contains(_present_queue.queue_family_index)) {
        queue_create_info.push_back(get_queue_create_info(_present_queue));
        unique_queue_family_indices.insert(_present_queue.queue_family_index);
    }
    if (device_requirements.require_compute &&
        !unique_queue_family_indices.contains(_compute_queue.queue_family_index)) {
        queue_create_info.push_back(get_queue_create_info(_compute_queue));
        unique_queue_family_indices.insert(_compute_queue.queue_family_index);
    }

    VkPhysicalDeviceFeatures2 enabled_features_2 = {
        .sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext    = nullptr,
        .features = {},
    };

    VkDeviceCreateInfo device_create_info = {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = &enabled_features_2,
        .flags                   = 0,
        .queueCreateInfoCount    = static_cast<uint32_t>(queue_create_info.size()),
        .pQueueCreateInfos       = &queue_create_info[0],
        .enabledLayerCount       = 0,        // deprecated. must be 0
        .ppEnabledLayerNames     = nullptr,  // deprecated. must be 0
        .enabledExtensionCount   = static_cast<uint32_t>(device_requirements.required_extensions.size()),
        .ppEnabledExtensionNames = &device_requirements.required_extensions[0],
        .pEnabledFeatures        = nullptr,
    };
    VK_CHECK_RESULT(vkCreateDevice(_physical_device, &device_create_info, _custom_allocator, &_device));
    CORE_LOG_INFO("(Vulkan) Successfully created logical device.")

    std::string device_creation_info;
    device_creation_info.append("\n(Vulkan) Device creation info:\n");
    device_creation_info.append(std::format("Number of queues: {}\n", queue_create_info.size()));
    device_creation_info.append("Enabled Extensions:\n");
    for (const char *const &extension_name : device_requirements.required_extensions) {
        device_creation_info.append(std::format("-- {}\n", extension_name));
    }
    device_creation_info.append("\n");

    CORE_LOG_INFO(device_creation_info.c_str());
    return true;
}

bool VulkanDevice::physical_device_meets_requirements(VkPhysicalDevice const &device,
                                                      VulkanQueryPhysicalDeviceRequirements const &requirements) {
    _physical_device_properties = {
        .sType      = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext      = nullptr,
        .properties = {},
    };
    vkGetPhysicalDeviceProperties2(device, &_physical_device_properties);
    const bool device_meets_device_type_requirements =
        (requirements.allow_dedicated &&
         _physical_device_properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) ||
        (requirements.allow_integrated &&
         _physical_device_properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
    if (!device_meets_device_type_requirements) {
        CORE_LOG_DEBUG("(Vulkan-Device) Device failed to meet device type requirements")
        return false;
    } else {
        CORE_LOG_INFO("(Vulkan-Device) Device meets device type requirements")
    }

    // Query for queue family properties
    u32 queue_family_properties_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(device, &queue_family_properties_count, nullptr);
    _queue_family_properties.assign(queue_family_properties_count,
                                    VkQueueFamilyProperties2{
                                        .sType                 = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2,
                                        .pNext                 = nullptr,
                                        .queueFamilyProperties = {},
                                    });
    vkGetPhysicalDeviceQueueFamilyProperties2(device, &queue_family_properties_count, &_queue_family_properties[0]);

    //
    // Query for queue families and check if they meet queue family requirements
    //
    std::string queue_info_text;
    queue_info_text.append("\n--- Queue family queue information ---\n")
        .append("N: number of queues\n")
        .append("G: Supports graphics\n")
        .append("C: Supports compute\n")
        .append("T: Supports transfer\n")
        .append("P: Supports present\n");
    CORE_LOG_DEBUG(queue_info_text.data());
    CORE_LOG_DEBUG("    | N | G | C | T | P ");

    for (u32 i = 0; i < queue_family_properties_count; ++i) {
        VkQueueFamilyProperties const &property = _queue_family_properties[i].queueFamilyProperties;

        bool has_graphics = (property.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
        bool has_compute  = (property.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
        bool has_transfer = (property.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0;

        VkBool32 supports_surface = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *_vulkan_surface, &supports_surface);

        if (has_graphics && requirements.require_graphics) {
            _graphics_queue = {
                .active             = true,
                .queue_family_index = i,
                .queue_count        = property.queueCount,
            };
            if (supports_surface == VK_TRUE && requirements.require_present) {
                _present_queue = {
                    .active             = true,
                    .queue_family_index = i,
                    .queue_count        = property.queueCount,
                };
            }
        }

        if (has_compute && requirements.require_compute) {
            _compute_queue = {
                .active             = true,
                .queue_family_index = i,
                .queue_count        = property.queueCount,
            };
        }

        if (has_transfer && requirements.require_transfer) {
            _transfer_queue = {
                .active             = true,
                .queue_family_index = i,
                .queue_count        = property.queueCount,
            };
        }

        if (!_present_queue.active && supports_surface && requirements.require_present) {
            _present_queue = {
                .active             = true,
                .queue_family_index = i,
                .queue_count        = property.queueCount,
            };
        }

        CORE_LOG_DEBUG("# %d | %d | %d | %d | %d | %d ",
                       i + 1,
                       property.queueCount,
                       has_graphics,
                       has_compute,
                       has_transfer,
                       supports_surface == VK_TRUE ? true : false);
    }

    const bool device_meets_queue_family_requirements = !(requirements.require_graphics && !_graphics_queue.active) &&
                                                        !(requirements.require_compute && !_compute_queue.active) &&
                                                        !(requirements.require_transfer && !_transfer_queue.active) &&
                                                        !(requirements.require_present && !_present_queue.active);
    if (!device_meets_queue_family_requirements) {
        CORE_LOG_DEBUG("(Vulkan-Device) Device does not meet queue family requirements.")
        return false;
    } else {
        CORE_LOG_DEBUG("(Vulkan-Device) Device queue family requirements met.")
    }

    u32 extension_count;
    std::vector<VkExtensionProperties> extension_properties;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
    extension_properties.resize(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, &extension_properties[0]);

    for (const char *const &extension_name : requirements.required_extensions) {
        bool has_extension = false;
        for (VkExtensionProperties const &extension_property : extension_properties) {
            if (std::strcmp(extension_name, extension_property.extensionName) == 0) {
                has_extension = true;
                break;
            }
        }

        if (!has_extension) {
            CORE_LOG_DEBUG("(Vulkan) Extension %s required but was not found. Device does not meet requirements.",
                           extension_name);
            return false;
        } else {
            CORE_LOG_DEBUG("(Vulkan) Required extension %s found on device.", extension_name)
        }
    }

    CORE_LOG_DEBUG("(Vulkan) Device meets extension requirements")
    if (!query_for_device_swapchain_support(device)) {
        CORE_LOG_DEBUG("(Vulkan-Device) Device does not have proper swapchain support")
    }

    return true;
}

bool VulkanDevice::query_for_device_swapchain_support(VkPhysicalDevice const &device) {
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = {
        .sType   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
        .pNext   = nullptr,
        .surface = *_vulkan_surface,
    };

    _surface_capabilities = {.sType               = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR,
                             .pNext               = nullptr,
                             .surfaceCapabilities = {}};
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilities2KHR(device, &surface_info, &_surface_capabilities));

    u32 surface_format_count;
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surface_info, &surface_format_count, nullptr));

    if (surface_format_count != 0) {
        _formats.resize(surface_format_count);
        for (u32 i = 0; i < surface_format_count; ++i) {
            _formats[i] = {.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR, .pNext = nullptr, .surfaceFormat = {}};
        }
        VK_CHECK_RESULT(
            vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surface_info, &surface_format_count, &_formats[0]));
    }

    u32 present_modes_count;
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, *_vulkan_surface, &present_modes_count, nullptr));
    if (present_modes_count != 0) {
        _present_modes.resize(present_modes_count);
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device,
                                                                  *_vulkan_surface,
                                                                  &present_modes_count,
                                                                  &_present_modes[0]));
    }

    bool has_swapchain_support = present_modes_count != 0 && surface_format_count != 0;
    if (has_swapchain_support) {
        CORE_LOG_DEBUG(
            "(Vulkan) Device meets swapchain support requirements. Device swapchain info:\nSurface formats "
            "(%d)\nPresent modes (%d)",
            surface_format_count,
            present_modes_count)
    }
    return has_swapchain_support;
}

static VkDeviceQueueCreateInfo get_queue_create_info(VulkanDeviceQueue const &queue) {
    PASTEL_ASSERT(queue.active);

    const int max_queues            = queue.queue_count;
    const float queue_priorities[2] = {0.5, 1.0};
    const u32 queues_to_use         = static_cast<uint32_t>(std::min(2, max_queues));
    const u32 queue_family_index    = static_cast<uint32_t>(queue.queue_family_index);

    CORE_LOG_INFO("Creating queue info for queue %d with %d number of queues", queue_family_index, queues_to_use);
    return VkDeviceQueueCreateInfo{
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = 0,
        .queueFamilyIndex = queue_family_index,
        .queueCount       = queues_to_use,
        .pQueuePriorities = queue_priorities,
    };
}

}  // namespace Pastel::Renderer::Vulkan
