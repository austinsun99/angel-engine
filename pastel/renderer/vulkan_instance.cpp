#include "vulkan_instance.h"
#include "core/logging/logger.h"
#include "pastel_types.h"
#include "vulkan_utils.h"
#include "core/logging/asserts.h"
#include <vulkan/vulkan_core.h>
#include <cstring>
#include <vector>

namespace Pastel::Renderer::Vulkan {

static bool check_required_extensions(std::vector<const char *> const &extension_names);
static bool check_required_layers(std::vector<const char *> const &layer_names);
static VkBool32 debug_utils_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                               VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                               const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                               void *pUserData);

bool vulkan_create_instance(VulkanCreateInstanceInfo const &create_info, VkInstance *out_instance) {
    CORE_LOG_INFO("(Vulkan Instance) Creating vulkan instance...");

    // Check API Version
    CORE_LOG_INFO("(Vulkan Instance) Checking if API version of (major.minor.patch) %d.%d.%d is met",
                  create_info.min_version_major,
                  create_info.min_version_minor,
                  create_info.min_version_patch);
    u32 api_version;
    VK_CHECK_RESULT(vkEnumerateInstanceVersion(&api_version));

    const u32 api_major = VK_API_VERSION_MAJOR(api_version);
    const u32 api_minor = VK_API_VERSION_MINOR(api_version);
    const u32 api_patch = VK_API_VERSION_PATCH(api_version);

    bool api_version_requirements_met = api_major >= create_info.min_version_major &&
                                        api_minor >= create_info.min_version_minor &&
                                        api_patch >= create_info.min_version_patch;

    CORE_LOG_INFO("(Vulkan Instance) Found API version of (major.minor.patch) %d.%d.%d",
                  api_major,
                  api_minor,
                  api_patch);
    if (!api_version_requirements_met) {
        CORE_LOG_FATAL("(Vulkan Instance) API version is not met.");
        return false;
    }

    // Define required extension and layer names
    std::vector<const char *> extension_names{
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
#if defined(PLATFORM_WINDOWS)
        "VK_KHR_win32_surface",
#elif defined(PLATFORM_LINUX)
        "VK_KHR_wayland_surface",
#endif
    };

    if (create_info.enable_debug_utils_extension) extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    std::vector<const char *> layer_names;
    if (create_info.enable_validation_layer) layer_names.push_back("VK_LAYER_KHRONOS_validation");

    // Query for availability of extension and layers
    if (check_required_extensions(extension_names) && check_required_layers(layer_names)) {
        CORE_LOG_INFO("(Vulkan Instance) All required extensions and layers found.")
    } else {
        CORE_LOG_ERROR("(Vulkan Instance) Failed to find required extensions and layers")
        return false;
    }

    void *vk_instance_p_next = nullptr;
    if (create_info.enable_debug_utils_extension) {
        CORE_LOG_INFO("(Vulkan Instance) Creating debug utils extension as extension to vulkan instance");

        VkDebugUtilsMessageSeverityFlagsEXT message_severify_flags =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        if (create_info.show_debug_info_messages)
            message_severify_flags |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        if (create_info.show_debug_versbose_messages)
            message_severify_flags |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

        VkDebugUtilsMessageTypeFlagsEXT message_type_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        if (create_info.show_debug_general_message) message_type_flags |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        if (create_info.show_debug_performance_message)
            message_type_flags |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        VkDebugUtilsMessengerCreateInfoEXT debug_utils_create_info{
            .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext           = nullptr,
            .flags           = 0,
            .messageSeverity = message_severify_flags,
            .messageType     = message_type_flags,
            .pfnUserCallback = debug_utils_messenger_callback,
            .pUserData       = nullptr,
        };
        vk_instance_p_next = &debug_utils_create_info;
    }

    VkApplicationInfo application_info{
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext              = nullptr,
        .pApplicationName   = create_info.application_name,
        .applicationVersion = create_info.application_version,
        .pEngineName        = create_info.engine_name,
        .engineVersion      = create_info.engine_version,
        .apiVersion         = VK_MAKE_API_VERSION(0, create_info.min_version_major, create_info.min_version_minor, 0),
    };

    VkInstanceCreateInfo instance_create_info{
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = vk_instance_p_next,
        .flags                   = 0,
        .pApplicationInfo        = &application_info,
        .enabledLayerCount       = static_cast<uint32_t>(layer_names.size()),
        .ppEnabledLayerNames     = &layer_names[0],
        .enabledExtensionCount   = static_cast<uint32_t>(extension_names.size()),
        .ppEnabledExtensionNames = &extension_names[0],
    };

    VK_CHECK_RESULT(vkCreateInstance(&instance_create_info, create_info.custom_allocator, out_instance));
    CORE_LOG_INFO("(Vulkan-Instance) Successfully created vulkan instance")
    return true;
}

static VkBool32 debug_utils_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                               VkDebugUtilsMessageTypeFlagsEXT message_types,
                                               const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                               void *user_data) {
    (void)message_types;
    (void)user_data;
    const char *const &message = callback_data->pMessage;
    switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            CORE_LOG_ERROR("(Vulkan-DEBUG_CALLBACK):\n%s", message);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            CORE_LOG_WARN("(Vulkan-DEBUG_CALLBACK):\n%s", message);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            CORE_LOG_INFO("(Vulkan-DEBUG_CALLBACK):\n%s", message);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            CORE_LOG_TRACE("(Vulkan-DEBUG_CALLBACK):\n%s", message);
            break;
        default:
            break;
    }
    return false;
}

static bool check_required_extensions(std::vector<const char *> const &extension_names) {
    CORE_LOG_INFO("(Vulkan Instance) Enumerating and checking for required extensions (%d)...", extension_names.size());
    u32 available_extension_count = 0;
    std::vector<VkExtensionProperties> available_extensions;
    VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, nullptr));
    available_extensions.resize(available_extension_count);
    VK_CHECK_RESULT(
        vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, &available_extensions[0]));

    for (const char *const &extension_name : extension_names) {
        CORE_LOG_DEBUG("(Vulkan Instance) Searching for extension %s", extension_name);
        bool found = false;
        for (VkExtensionProperties const &extension_property : available_extensions) {
            if (std::strcmp(extension_name, extension_property.extensionName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            CORE_LOG_ERROR("(Vulkan Instance) Could not find extension.");
            return false;
        } else {
            CORE_LOG_DEBUG("Extension found.")
        }
    }

    return true;
}

static bool check_required_layers(std::vector<const char *> const &layer_names) {
    CORE_LOG_INFO("(Vulkan Instance) Enumerating and checking for required layers (%d)...", layer_names.size());
    u32 available_layer_count = 0;
    std::vector<VkLayerProperties> available_layers;
    VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr));
    available_layers.resize(available_layer_count);
    VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&available_layer_count, &available_layers[0]));
    for (const char *const &layer_name : layer_names) {
        CORE_LOG_DEBUG("(Vulkan Instance) Searching for layer %s", layer_name);
        bool found = false;
        for (VkLayerProperties const &layer_property : available_layers) {
            if (std::strcmp(layer_name, layer_property.layerName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            CORE_LOG_ERROR("(Vulkan Instance) Could not find layer.");
            return false;
        } else {
            CORE_LOG_DEBUG("Layer found.")
        }
    }
    return true;
}

}  // namespace Pastel::Renderer::Vulkan
