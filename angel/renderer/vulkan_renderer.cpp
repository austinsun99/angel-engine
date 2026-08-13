#include "vulkan_renderer.h"
#include <vulkan/vulkan_core.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include "renderer/vector.hpp"
#include "renderer/vulkan_constructor.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/logging/logger.h"
#include "renderer/vulkan_command_buffer.h"
#include "renderer/vulkan_device.h"
#include "renderer/vulkan_swapchain.h"
#include "renderer/vulkan_synchronization.hpp"
#include "vulkan_instance.h"

namespace angel::renderer::vulkan {
VulkanRenderer::~VulkanRenderer() {
    vkDeviceWaitIdle(_device.device());

    vkDestroyDescriptorPool(_device.device(), _descriptor_pool, _custom_allocator);
    vkDestroyDescriptorSetLayout(_device.device(), _descriptor_set_layout, _custom_allocator);
    for (u64 i = 0; i < _uniform_buffers.size(); ++i) {
        vkDestroyBuffer(_device.device(), _uniform_buffers[i], _custom_allocator);
        vkFreeMemory(_device.device(), _uniform_buffers_memory[i], _custom_allocator);
    }

    vkFreeMemory(_device.device(), _index_buffer_memory, _custom_allocator);
    vkDestroyBuffer(_device.device(), _index_buffer, _custom_allocator);
    vkFreeMemory(_device.device(), _vertex_buffer_memory, _custom_allocator);
    vkDestroyBuffer(_device.device(), _vertex_buffer, _custom_allocator);

    AL_CORE_INFO("(Vulkan) Destroying vulkan graphics pipeline");
    _graphics_pipeline.destroy();

    AL_CORE_INFO("(Vulkan) Destroying vulkan sync objects");
    _sync_objects.destroy();

    AL_CORE_INFO("(Vulkan) Destroying vulkan swapchain");
    _swapchain.destroy_swapchain();

    AL_CORE_INFO("(Vulkan) Destroying vulkan device");
    _device.destroy_graphics_command_pool();
    _device.destroy_device();

    AL_CORE_INFO("(Vulkan) Destroying vulkan surface");
    vkDestroySurfaceKHR(_instance, _surface, _custom_allocator);

    AL_CORE_INFO("(Vulkan) Destroying vulkan instance");
    vkDestroyInstance(_instance, _custom_allocator);
}

void VulkanRenderer::init(platform::Window const &window_state, VkAllocationCallbacks *custom_allocator) {
    _window_state                                 = &window_state;
    _custom_allocator                             = custom_allocator;
    VulkanCreateInstanceInfo create_instance_info = {
        .custom_allocator = _custom_allocator,

        .application_name    = "Angel Game Engine",
        .application_version = VK_MAKE_VERSION(0, 1, 0),
        .engine_name         = "Angel Game Engine",
        .engine_version      = VK_MAKE_VERSION(0, 1, 0),

        .min_version_major = 1,
        .min_version_minor = 4,
        .min_version_patch = 0,

#if defined(ANGEL_DEBUG)
        .enable_debug_utils_extension = true,
#elif defined(ANGEL_RELEASE)
        .enable_debug_utils_extension = false,
#endif
        .show_debug_info_messages     = false,
        .show_debug_versbose_messages = false,
        .enable_validation_layer      = true,
    };

    if (!vulkan_create_instance(create_instance_info, &_instance)) {
        AL_CORE_FATAL("(Vulkan) Failed to create vulkan instance. Check log for details.")
    };

    if (!_window_state->create_vulkan_surface(_instance, _custom_allocator, &_surface)) {
        AL_CORE_FATAL("(Vulkan) Failed to create vulkan surface. Check log for details.")
    } else {
        AL_CORE_INFO("(Vulkan) Successfully obtained vulkan surface.")
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
            AL_CORE_INFO("(Vulkan) Found suitable physical device.")
            break;
        }
    }

    _device.setup_device(_instance, _surface, _custom_allocator, selected_device, requirements, properties);
    _device.create_logical_device();

    _device.create_graphics_command_pool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    _current_framebuffer_width  = _window_state->framebuffer_width();
    _current_framebuffer_height = _window_state->framebuffer_height();
    _swapchain.init(&_device, _custom_allocator, _surface);
    _swapchain.create_swapchain(_current_framebuffer_width, _current_framebuffer_height);

    // Vertex, index, and uniform buffer creation
    const VkDeviceSize vertex_buffer_size = sizeof(vertices[0]) * vertices.size();
    const VkDeviceSize index_buffer_size  = sizeof(indices[0]) * indices.size();

    const bool vertex_buffer_creation_success =
        create_buffer(&_device,
                      _device.device_properties().graphics_queue_index,
                      vertex_buffer_size,
                      VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_2_TRANSFER_DST_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      &_vertex_buffer,
                      &_vertex_buffer_memory);
    const bool index_buffer_creation_success =
        create_buffer(&_device,
                      _device.device_properties().graphics_queue_index,
                      index_buffer_size,
                      VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_2_TRANSFER_DST_BIT,
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                      &_index_buffer,
                      &_index_buffer_memory);

    if (!vertex_buffer_creation_success || !index_buffer_creation_success) {
        AL_CORE_ERROR("(Vulkan-Swapchain) Failed to create vertex buffer.")
    };
    create_staging_buffer_and_stage(&_device, vertex_buffer_size, vertices.data(), _vertex_buffer, _custom_allocator);
    create_staging_buffer_and_stage(&_device, index_buffer_size, indices.data(), _index_buffer, _custom_allocator);

    const VkDeviceSize buffer_size = sizeof(UniformBuffer);
    _uniform_buffers.resize(FRAMES_IN_FLIGHT);
    _uniform_buffers_memory.resize(FRAMES_IN_FLIGHT);
    _uniform_buffers_map.resize(FRAMES_IN_FLIGHT);

    for (u64 i = 0; i < FRAMES_IN_FLIGHT; ++i) {
        create_buffer(&_device,
                      _device.device_properties().graphics_queue_index,
                      buffer_size,
                      VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      &_uniform_buffers[i],
                      &_uniform_buffers_memory[i],
                      _custom_allocator);
        vkMapMemory(_device.device(), _uniform_buffers_memory[i], 0, buffer_size, 0, &_uniform_buffers_map[i]);
    }

    create_descriptor_layout_and_pool(_device.device(),
                                      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                      VK_SHADER_STAGE_VERTEX_BIT,
                                      FRAMES_IN_FLIGHT,
                                      &_descriptor_set_layout,
                                      &_descriptor_pool,
                                      _custom_allocator);
    allocate_descriptor_sets(_device.device(), _descriptor_set_layout, _descriptor_pool, _descriptor_sets);
    for (u64 i = 0; i < FRAMES_IN_FLIGHT; ++i) {
        VkDescriptorBufferInfo buffer_info{
            .buffer = _uniform_buffers[i],
            .offset = 0,
            .range  = sizeof(UniformBuffer),
        };

        VkWriteDescriptorSet write_descriptor_set{
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext            = nullptr,
            .dstSet           = _descriptor_sets[i],
            .dstBinding       = 0,
            .dstArrayElement  = 0,
            .descriptorCount  = 1,
            .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo       = nullptr,
            .pBufferInfo      = &buffer_info,
            .pTexelBufferView = nullptr,
        };
        vkUpdateDescriptorSets(_device.device(), 1, &write_descriptor_set, 0, nullptr);
    }

    _graphics_command_buffers.resize(
        _swapchain.images().size(),
        VulkanCommandBuffer(_device.graphics_command_pool(), _device.device(), _custom_allocator));
    for (auto &buffer : _graphics_command_buffers) {
        buffer.create(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }

    _sync_objects.create(_swapchain.images().size(), true, _device.device(), _custom_allocator);

    _graphics_pipeline.init(&_device, &_swapchain, _custom_allocator);
    _graphics_pipeline.create(_descriptor_set_layout);
}

void VulkanRenderer::update_start() {
    if (_current_framebuffer_width != _window_state->framebuffer_width()) {
        _should_recreate_swapchain = true;
        _current_framebuffer_width = _window_state->framebuffer_width();
    }
    if (_current_framebuffer_height != _window_state->framebuffer_height()) {
        _should_recreate_swapchain  = true;
        _current_framebuffer_height = _window_state->framebuffer_height();
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
        std::min(_current_framebuffer_width / 4000.f, 1.f),
        std::min(_current_framebuffer_height / 4000.f, 1.f),
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

    update_uniform_buffer(_current_frame);

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

    if (_vertex_buffer == VK_NULL_HANDLE) {
        AL_CORE_ERROR("checkin")
    }
    const VkDeviceSize offsets[] = {0};
    const VkBuffer buffers[]     = {_vertex_buffer};
    vkCmdBindVertexBuffers(buffer.handle(), 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(buffer.handle(), _index_buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdSetViewport(buffer.handle(), 0, 1, &viewport);
    vkCmdSetScissor(buffer.handle(), 0, 1, &scissor);

    vkCmdBindDescriptorSets(buffer.handle(),
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            _graphics_pipeline.layout(),
                            0,
                            1,
                            &_descriptor_sets[_current_frame],
                            0,
                            nullptr);
    vkCmdDrawIndexed(buffer.handle(), indices.size(), 1, 0, 0, 0);
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

// @todo: update to use window state time
void VulkanRenderer::update_uniform_buffer(u32 current_image) {
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto current_time      = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

    UniformBuffer ubo{};

    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj  = glm::perspective(glm::radians(45.0f),
                                 _current_framebuffer_width / (float)_current_framebuffer_height,
                                 0.1f,
                                 10.0f);
    ubo.proj[1][1] *= -1;
    std::memcpy(_uniform_buffers_map[current_image], &ubo, sizeof(ubo));
}

}  // namespace angel::renderer::vulkan
