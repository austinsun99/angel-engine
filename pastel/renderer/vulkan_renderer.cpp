#include "vulkan_renderer.h"
#include <vulkan/vulkan_core.h>
#include "core/logging/logger.h"
#include "renderer/vulkan_device.h"
#include "vulkan_instance.h"

namespace Pastel::Renderer::Vulkan {
VulkanRenderer::VulkanRenderer(Platform::WindowState const &window_state) : _window_state(window_state) {
    _custom_allocator = nullptr;
}

VulkanRenderer::~VulkanRenderer() {
    CORE_LOG_INFO("(Vulkan) Destroying vulkan device");
    _vulkan_device.destroy_device();

    CORE_LOG_INFO("(Vulkan) Destroying vulkan surface");
    vkDestroySurfaceKHR(_vulkan_instance, _vulkan_surface, _custom_allocator);

    CORE_LOG_INFO("(Vulkan) Destroying vulkan instance");
    vkDestroyInstance(_vulkan_instance, _custom_allocator);
}

void VulkanRenderer::start() {
    VulkanCreateInstanceInfo create_instance_info = {
        .custom_allocator = _custom_allocator,

        .application_name    = "Pastel Game Engine",
        .application_version = VK_MAKE_VERSION(0, 1, 0),
        .engine_name         = "Pastel Game Engine",
        .engine_version      = VK_MAKE_VERSION(0, 1, 0),

        .min_version_major = 1,
        .min_version_minor = 4,
        .min_version_patch = 0,

#if defined(PASTEL_DEBUG)
        .enable_debug_utils_extension = true,
#elif defined(PASTEL_RELEASE)
        .enable_debug_utils_extension = false,
#endif
        .show_debug_info_messages     = false,
        .show_debug_versbose_messages = false,
        .enable_validation_layer      = true,
    };

    if (!vulkan_create_instance(create_instance_info, &_vulkan_instance)) {
        CORE_LOG_FATAL("(Vulkan) Failed to create vulkan instance. Check log for details.")
    };

    if (!_window_state.create_vulkan_surface(_vulkan_instance, _custom_allocator, &_vulkan_surface)) {
        CORE_LOG_FATAL("(Vulkan) Failed to create vulkan surface. Check log for details.")
    } else {
        CORE_LOG_INFO("(Vulkan) Successfully obtained vulkan surface.")
    }

    std::vector<VkPhysicalDevice> physical_devices;
    vulkan_get_physical_devices(_vulkan_instance, physical_devices);

    VulkanPhysicalDeviceRequirements requirements;
    VulkanPhysicalDeviceProperties properties;
    VkPhysicalDevice selected_device;
    for (VkPhysicalDevice const &device : physical_devices) {
        vulkan_get_physical_device_properties(device, _vulkan_surface, properties);
        if (vulkan_physical_device_meets_requirements(properties, requirements, create_instance_info)) {
            selected_device = device;
            CORE_LOG_INFO("(Vulkan) Found suitable physical device.")
            break;
        }
    }

    _vulkan_device
        .setup_device(_vulkan_instance, _vulkan_surface, _custom_allocator, selected_device, requirements, properties);
    _vulkan_device.create_logical_device();
    // _vulkan_device.create_logical_device();
}
}  // namespace Pastel::Renderer::Vulkan
