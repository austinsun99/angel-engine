#include "vulkan_renderer.h"
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "core/logging/logger.h"
#include "renderer/vulkan_command_buffer.h"
#include "renderer/vulkan_device.h"
#include "renderer/vulkan_synchronization.hpp"
#include "vulkan_instance.h"

namespace Pastel::Renderer::Vulkan {
VulkanRenderer::~VulkanRenderer() {
    vkDeviceWaitIdle(_device.device());
    for (VulkanSyncObject &sync : _sync_objects) {
        sync.destroy();
    }

    CORE_LOG_INFO("(Vulkan) Destroying vulkan swapchain");
    _swapchain.destroy_swapchain();

    CORE_LOG_INFO("(Vulkan) Destroying vulkan device");
    _device.destroy_graphics_command_pool();
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
    _device.create_graphics_command_pool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    _swapchain.init(&_device, _custom_allocator, _surface);
    _swapchain.create_swapchain(_window_state.framebuffer_width(), _window_state.framebuffer_height());

    _graphics_command_buffers.resize(
        _swapchain.images().size(),
        VulkanCommandBuffer(_device.graphics_command_pool(), _device.device(), _custom_allocator));
    for (auto& buffer : _graphics_command_buffers) {
        buffer.create(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }

    _sync_objects.resize(FRAMES_IN_FLIGHT, VulkanSyncObject(_device.device(), _custom_allocator));
    for (auto &obj : _sync_objects) {
        // Initially set to signalled so we dont wait on the first frame
        obj.create(true);
    }
}

void VulkanRenderer::update_start() {
    VulkanCommandBuffer buffer = _graphics_command_buffers[_current_frame];
    VulkanSyncObject sync_obj  = _sync_objects[_current_frame];
    sync_obj.wait_fence(UINT64_MAX);
    sync_obj.reset_fence();

    u32 image_index = 0;
    _swapchain.acquire_next_image(UINT64_MAX, sync_obj.image_available_sem(), nullptr, &image_index);

    const VkClearColorValue clear_colour = {.float32{
        1.0f,
        0.0f,
        0.0f,
        0.0,
    }};

    const VkImageSubresourceRange subresource_range{
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = 0,
        .levelCount     = 1,
        .baseArrayLayer = 0,
        .layerCount     = 1,
    };

    buffer.reset();
    buffer.begin(true, false, false);
    vkCmdClearColorImage(buffer.handle(),
                         _swapchain.images()[image_index],
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         &clear_colour,
                         1,
                         &subresource_range);
    buffer.end();

    VkPipelineStageFlags stage_flags = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    VkSubmitInfo submit_info = {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = nullptr,
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &sync_obj.image_available_sem(),
        .pWaitDstStageMask    = &stage_flags,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &buffer.handle(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &sync_obj.queue_complete_sem(),
    };
    vkQueueSubmit(_device.graphics_queue(), 1, &submit_info, sync_obj.fence());

    VkResult result = _swapchain.present(sync_obj.queue_complete_sem(), image_index);
    (void)result;

    _current_frame++;
    _current_frame %= FRAMES_IN_FLIGHT;
}

void VulkanRenderer::update_end() {
}

}  // namespace Pastel::Renderer::Vulkan
