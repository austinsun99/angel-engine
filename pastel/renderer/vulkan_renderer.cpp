#include "vulkan_renderer.h"
#include <vulkan/vulkan_core.h>
#include "core/logging/asserts.h"
#include "core/logging/logger.h"
#include "renderer/vulkan_device.h"
#include "vulkan_instance.h"

namespace Pastel::Renderer::Vulkan {
VulkanRenderer::VulkanRenderer(Platform::WindowState const &window_state) : _window_state(window_state) {
    _custom_allocator = nullptr;
}

VulkanRenderer::~VulkanRenderer() {
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

    _vulkan_device.init_device(&_vulkan_instance, &_vulkan_surface, _custom_allocator);

    VulkanQueryPhysicalDeviceRequirements query_physical_device_requirements{
        .dynamic_rendering = true,
    };
    _vulkan_device.query_for_physical_device(query_physical_device_requirements);
    _vulkan_device.create_logical_device(query_physical_device_requirements);
}
}  // namespace Pastel::Renderer::Vulkan
