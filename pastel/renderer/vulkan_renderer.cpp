#include "vulkan_renderer.h"
#include <vulkan/vulkan_core.h>
#include "core/logging/logger.h"
#include "renderer/vulkan_device.h"
#include "vulkan_instance.h"

namespace Pastel::Renderer::Vulkan {
VulkanRenderer::~VulkanRenderer() {
    CORE_LOG_INFO("(Vulkan) Destroying vulkan device");
    _device.destroy_device();

    CORE_LOG_INFO("(Vulkan) Destroying vulkan surface");
    vkDestroySurfaceKHR(_instance, _surface, _custom_allocator);

    CORE_LOG_INFO("(Vulkan) Destroying vulkan instance");
    vkDestroyInstance(_instance, _custom_allocator);
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

    if (!vulkan_create_instance(create_instance_info, &_instance)) {
        CORE_LOG_FATAL("(Vulkan) Failed to create vulkan instance. Check log for details.")
    };

    if (!_window_state.create_vulkan_surface(_instance, _custom_allocator, &_surface)) {
        CORE_LOG_FATAL("(Vulkan) Failed to create vulkan surface. Check log for details.")
    } else {
        CORE_LOG_INFO("(Vulkan) Successfully obtained vulkan surface.")
    }

    std::vector<VkPhysicalDevice> physical_devices;
    vulkan_get_physical_devices(_instance, physical_devices);

    VulkanPhysicalDeviceRequirements requirements;
    VulkanPhysicalDeviceProperties properties;
    VkPhysicalDevice selected_device;
    for (VkPhysicalDevice const &device : physical_devices) {
        vulkan_get_physical_device_properties(device, _surface, properties);
        if (vulkan_physical_device_meets_requirements(properties, requirements, create_instance_info)) {
            selected_device = device;
            CORE_LOG_INFO("(Vulkan) Found suitable physical device.")
            break;
        }
    }

    _device.setup_device(_instance, _surface, _custom_allocator, selected_device, requirements, properties);
    _device.create_logical_device();

    _swapchain.init(&_device, _custom_allocator, _surface);
    _swapchain.create_swapchain(_window_state.framebuffer_width(), _window_state.framebuffer_height());
}
}  // namespace Pastel::Renderer::Vulkan
