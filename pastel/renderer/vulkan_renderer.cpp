#include "vulkan_renderer.h"
#include <vulkan/vulkan_core.h>
#include "core/logging/logger.h"
#include "vulkan_instance.h"

namespace Pastel::Renderer::Vulkan {
VulkanRenderer::VulkanRenderer(Platform::WindowState const &window_state) : _window_state(window_state) {
}

VulkanRenderer::~VulkanRenderer() {
}

void VulkanRenderer::start() {
    VulkanCreateInstanceInfo create_instance_info = {
        .custom_allocator = nullptr,

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
        .show_debug_info_messages = false,
        .show_debug_versbose_messages = false,
        .enable_validation_layer = true,
    };

    if (!vulkan_create_instance(create_instance_info, &_vulkan_instance)) {
        CORE_LOG_FATAL("(Vulkan) Failed to create vulkan instnace. Check log for details.")
    };
}  // namespace Pastel::Renderer::Vulkan

}  // namespace Pastel::Renderer::Vulkan
