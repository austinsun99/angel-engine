#include "vulkan_renderer.h"
#include <vulkan/vulkan_core.h>
#include <cstddef>
#include <cstdint>
#include "core/logging/logger.h"
#include "renderer/vulkan_command_buffer.h"
#include "renderer/vulkan_device.h"
#include "renderer/vulkan_synchronization.hpp"
#include "vulkan_instance.h"

namespace Pastel::Renderer::Vulkan {
VulkanRenderer::~VulkanRenderer() {
    vkDeviceWaitIdle(_device.device());
    _sync_objects.destroy();

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

    _current_framebuffer_width  = _window_state.framebuffer_width();
    _current_framebuffer_height = _window_state.framebuffer_height();
    _swapchain.init(&_device, _custom_allocator, _surface);
    _swapchain.create_swapchain(_current_framebuffer_width, _current_framebuffer_height);

    _graphics_command_buffers.resize(
        _swapchain.images().size(),
        VulkanCommandBuffer(_device.graphics_command_pool(), _device.device(), _custom_allocator));
    for (auto &buffer : _graphics_command_buffers) {
        buffer.create(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }

    _sync_objects.create(_swapchain.images().size(), true, _device.device(), _custom_allocator);

    _graphics_pipeline.init(&_device, &_swapchain, _custom_allocator);
    _graphics_pipeline.create();
}

void VulkanRenderer::update_start() {
    // const u32 present_queue_index = _device.device_properties().present_queue_index;

    if (_current_framebuffer_width != _window_state.framebuffer_width()) {
        _should_recreate_swapchain = true;
        _current_framebuffer_width = _window_state.framebuffer_width();
    }
    if (_current_framebuffer_height != _window_state.framebuffer_height()) {
        _should_recreate_swapchain  = true;
        _current_framebuffer_height = _window_state.framebuffer_height();
    }

    if (_should_recreate_swapchain) {
        vkDeviceWaitIdle(_device.device());
        _device.requery_swapchain_info();
        _swapchain.destroy_swapchain();
        _swapchain.create_swapchain(_current_framebuffer_width, _current_framebuffer_height);
        _should_recreate_swapchain = false;
    }

    _sync_objects.wait_fence(_current_frame, UINT64_MAX);
    _sync_objects.reset_fence(_current_frame);

    u32 image_index = 0;
    _swapchain.acquire_next_image(UINT64_MAX, _sync_objects.image_available_sem(_current_frame), nullptr, &image_index);

    VulkanCommandBuffer buffer = _graphics_command_buffers[image_index];

    const VkClearColorValue clear_colour = {.float32{
        1.0f,
        _current_framebuffer_width / 4000.0f,
        _current_framebuffer_height / 4000.0f,
        0.0,
    }};

    VkViewport viewport{
        .x        = 0.0f,
        .y        = 0.0f,
        .width    = static_cast<float>(_current_framebuffer_width),
        .height   = static_cast<float>(_current_framebuffer_height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor{
        .offset{0, 0},
        .extent{.width = _current_framebuffer_width, .height = _current_framebuffer_height},
    };

    VkClearValue clear_value{};
    clear_value.color = clear_colour;

    VkRenderingAttachmentInfo attachment_info{};
    attachment_info.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    attachment_info.imageView   = _swapchain.image_views()[image_index];
    attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment_info.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_info.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_info.clearValue  = clear_value;

    VkRenderingInfo rendering_info{};
    rendering_info.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.renderArea           = scissor;
    rendering_info.layerCount           = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments    = &attachment_info;

    buffer.reset();
    buffer.begin(true, false, false);
    buffer.transition_image_layout(_swapchain.images()[image_index],
                                   VK_IMAGE_LAYOUT_UNDEFINED,
                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                   {},
                                   VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                   VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

    vkCmdBeginRendering(buffer.handle(), &rendering_info);
    vkCmdBindPipeline(buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline.handle());

    vkCmdSetViewport(buffer.handle(), 0, 1, &viewport);
    vkCmdSetScissor(buffer.handle(), 0, 1, &scissor);

    vkCmdDraw(buffer.handle(), 3, 1, 0, 0);

    vkCmdEndRendering(buffer.handle());

    buffer.transition_image_layout(_swapchain.images()[image_index],
                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                   VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                   {},
                                   VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);
    buffer.end();

    VkPipelineStageFlags stage_flags = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    VkSubmitInfo submit_info = {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = nullptr,
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &_sync_objects.image_available_sem(_current_frame),
        .pWaitDstStageMask    = &stage_flags,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &buffer.handle(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &_sync_objects.queue_complete_sem(image_index),
    };
    vkQueueSubmit(_device.graphics_queue(), 1, &submit_info, _sync_objects.fence(_current_frame));

    VkResult result = _swapchain.present(_sync_objects.queue_complete_sem(image_index), image_index);
    if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
        _should_recreate_swapchain = true;
    }

    _current_frame++;
    _current_frame %= FRAMES_IN_FLIGHT;
}

void VulkanRenderer::update_end() {
}

}  // namespace Pastel::Renderer::Vulkan
