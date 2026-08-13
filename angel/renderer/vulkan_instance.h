#pragma once

#include <vulkan/vulkan_core.h>
#include "angel_types.h"
namespace angel::renderer::vulkan {

struct VulkanCreateInstanceInfo {
    VkAllocationCallbacks *const &custom_allocator;

    const char *application_name;
    u32 application_version;
    const char *engine_name;
    u32 engine_version;

    u32 min_version_major;
    u32 min_version_minor;
    u32 min_version_patch;

    bool enable_debug_utils_extension;
    bool show_debug_info_messages     = false;
    bool show_debug_versbose_messages = false;

    bool show_debug_performance_message = false;
    bool show_debug_general_message     = false;

    bool enable_validation_layer;
};

bool vulkan_create_instance(VulkanCreateInstanceInfo const &create_info, VkInstance *out_instance);
bool vulkan_ceate_debug_messenger();

}  // namespace angel::renderer::vulkan
