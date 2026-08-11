#include <vulkan/vulkan_core.h>
#include <algorithm>
#include <cstring>
#include "vulkan_swapchain.h"
#include "core/logging/logger.h"
#include "core/logging/asserts.h"
#include "renderer/vector.hpp"
#include "renderer/vulkan_renderer.h"
#include "vulkan_utils.h"

namespace Pastel::Renderer::Vulkan {
bool VulkanSwapchain::create_swapchain(const u32 framebuffer_width, const u32 framebuffer_height) {
    if (!_initialized) {
        CORE_LOG_ERROR("(Vulkan-Swapchain) Swapchain has not been initialized. Initialize the swapchain first.")
        return false;
    }

    VulkanPhysicalDeviceProperties const &dev_properties    = _device->device_properties();
    std::vector<VkSurfaceFormat2KHR> const &surface_formats = dev_properties.surface_formats;

    PASTEL_ASSERT(surface_formats.size() > 0);

    // Query for format and colourspace
    _selected_surface_format = surface_formats[0].surfaceFormat;
    for (VkSurfaceFormat2KHR const &fmt : surface_formats) {
        // @todo: allow customization of wanted format/colourspace
        if (fmt.surfaceFormat.format == VK_FORMAT_R8G8B8A8_SRGB &&
            fmt.surfaceFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
            _selected_surface_format = fmt.surfaceFormat;
            break;
        }
    }

    // Query for present mode
    VkPresentModeKHR selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (VkPresentModeKHR const &present : dev_properties.surface_present_modes) {
        if (present == VK_PRESENT_MODE_MAILBOX_KHR) {
            selected_present_mode = present;
            break;
        }
    }

    // Query for extents
    VkSurfaceCapabilitiesKHR const &surface_capabilities = dev_properties.surface_capabilities.surfaceCapabilities;
    _current_extent                                      = surface_capabilities.currentExtent;
    if (surface_capabilities.currentExtent.width == std::numeric_limits<u32>::max()) {
        _current_extent = {.width  = std::clamp(framebuffer_width,
                                                surface_capabilities.minImageExtent.width,
                                                surface_capabilities.maxImageExtent.width),
                           .height = std::clamp(framebuffer_height,
                                                surface_capabilities.minImageExtent.height,
                                                surface_capabilities.maxImageExtent.height)};
    }

    // Query and set minimum image count
    u32 swapchain_image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount != 0 && swapchain_image_count > surface_capabilities.maxImageCount) {
        swapchain_image_count = surface_capabilities.maxImageCount;
    }

    // Set sharing mode based on graphics and present queue family index
    VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
    u32 index_count            = 1;
    std::vector<u32> family_indices{
        dev_properties.graphics_queue_index,
    };

    if (dev_properties.present_queue_index != dev_properties.graphics_queue_index) {
        sharing_mode = VK_SHARING_MODE_CONCURRENT;
        index_count  = 2;
        family_indices.push_back(dev_properties.present_queue_index);
    }

    // Create the swapchain
    const VkSwapchainCreateInfoKHR swapchain_create_info{
        .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext                 = nullptr,
        .flags                 = 0,
        .surface               = _surface,
        .minImageCount         = swapchain_image_count,
        .imageFormat           = _selected_surface_format.format,
        .imageColorSpace       = _selected_surface_format.colorSpace,
        .imageExtent           = _current_extent,
        .imageArrayLayers      = 1,
        .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode      = sharing_mode,
        .queueFamilyIndexCount = index_count,
        .pQueueFamilyIndices   = &family_indices[0],
        .preTransform          = surface_capabilities.currentTransform,
        .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode           = selected_present_mode,
        .clipped               = VK_TRUE,
        .oldSwapchain          = VK_NULL_HANDLE,
    };

    VK_CHECK_RESULT(vkCreateSwapchainKHR(_device->device(), &swapchain_create_info, _custom_allocator, &_handle));

    u32 image_count = 0;
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(_device->device(), _handle, &image_count, nullptr));
    PASTEL_ASSERT(swapchain_image_count == image_count)
    _images.resize(image_count);
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(_device->device(), _handle, &image_count, &_images[0]));

    if (!create_image_views()) {
        return false;
    }

    return true;
}

bool VulkanSwapchain::create_index_buffer() {
    const VkBufferUsageFlags2CreateInfo staging_usage_flags{
        .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO,
        .pNext = nullptr,
        .usage = VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT,
    };

    const VkBufferUsageFlags2CreateInfo index_usage_flags{
        .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO,
        .pNext = nullptr,
        .usage = VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_2_TRANSFER_DST_BIT,
    };
    const VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();
    const VkMemoryPropertyFlags property_flags =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkBuffer staging_buf;
    VkDeviceMemory staging_buf_mem;

    if (!_device->create_buffer(buffer_size,
                                staging_usage_flags,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                &staging_buf,
                                &staging_buf_mem)) {
        CORE_LOG_ERROR("(Vulkan-Swapchain) Failed to create staging buffer for index buffer.")
        return false;
    }

    if (!_device
             ->create_buffer(buffer_size, index_usage_flags, property_flags, &_index_buffer, &_index_buffer_memory)) {
        CORE_LOG_ERROR("(Vulkan-Swapchain) Failed to create index buffer.")
        return false;
    };

    void *data;
    VK_CHECK_RESULT(vkMapMemory(_device->device(), staging_buf_mem, 0, buffer_size, 0, &data));
    std::memcpy(data, &indices[0], buffer_size);
    vkUnmapMemory(_device->device(), staging_buf_mem);

    _device->copy_buffer(_index_buffer, staging_buf, buffer_size);
    vkDestroyBuffer(_device->device(), staging_buf, _custom_allocator);
    vkFreeMemory(_device->device(), staging_buf_mem, _custom_allocator);
    return true;
}

bool VulkanSwapchain::create_uniform_buffers() {
    const VkBufferUsageFlags2CreateInfo create_flags{
        .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO,
        .pNext = nullptr,
        .usage = VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT,
    };

    const VkDeviceSize buffer_size = sizeof(UniformBuffer);
    _uniform_buffers.resize(FRAMES_IN_FLIGHT);
    _uniform_buffers_memory.resize(FRAMES_IN_FLIGHT);
    _uniform_buffers_map.resize(FRAMES_IN_FLIGHT);

    for (u64 i = 0; i < FRAMES_IN_FLIGHT; ++i) {
        _device->create_buffer(buffer_size,
                               create_flags,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               &_uniform_buffers[i],
                               &_uniform_buffers_memory[i]);
        vkMapMemory(_device->device(), _uniform_buffers_memory[i], 0, buffer_size, 0, &_uniform_buffers_map[i]);
    }

    return true;
}

bool VulkanSwapchain::create_descriptor_set_layout_and_pool() {
    VkDescriptorSetLayoutBinding ubo_layout_binding{
        .binding            = 0,
        .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount    = 1,
        .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr,
    };

    VkDescriptorSetLayoutCreateInfo layout_create_info{
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = nullptr,
        .flags        = 0,
        .bindingCount = 1,
        .pBindings    = &ubo_layout_binding,
    };

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(_device->device(),
                                                &layout_create_info,
                                                _custom_allocator,
                                                &_descriptor_set_layout));

    VkDescriptorPoolSize pool_size{
        .type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = FRAMES_IN_FLIGHT,
    };

    VkDescriptorPoolCreateInfo pool_create_info{
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = 0,
        .maxSets       = FRAMES_IN_FLIGHT,
        .poolSizeCount = 1,
        .pPoolSizes    = &pool_size,
    };

    VK_CHECK_RESULT(vkCreateDescriptorPool(_device->device(), &pool_create_info, _custom_allocator, &_descriptor_pool));

    std::vector<VkDescriptorSetLayout> layouts(FRAMES_IN_FLIGHT, _descriptor_set_layout);

    VkDescriptorSetAllocateInfo set_allocate_info{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = _descriptor_pool,
        .descriptorSetCount = FRAMES_IN_FLIGHT,
        .pSetLayouts        = &layouts[0],
    };

    _descriptor_sets.resize(FRAMES_IN_FLIGHT);
    VK_CHECK_RESULT(vkAllocateDescriptorSets(_device->device(), &set_allocate_info, &_descriptor_sets[0]));

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
        vkUpdateDescriptorSets(_device->device(), 1, &write_descriptor_set, 0, nullptr);
    }

    return true;
}

VkResult VulkanSwapchain::present(VkSemaphore const &render_complete_sem, u32 image_index) {
    const VkPresentInfoKHR present_info{
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext              = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &render_complete_sem,
        .swapchainCount     = 1,
        .pSwapchains        = &_handle,
        .pImageIndices      = &image_index,
        .pResults           = nullptr,
    };

    return vkQueuePresentKHR(_device->present_queue(), &present_info);
}

bool VulkanSwapchain::create_image_views() {
    PASTEL_ASSERT(_images.size() > 0);

    _image_views.resize(_images.size());
    VkImageViewCreateInfo view_create_info{
        .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = 0,
        .image    = nullptr,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format   = _selected_surface_format.format,
        .components =
            {
                .r = VK_COMPONENT_SWIZZLE_R,
                .g = VK_COMPONENT_SWIZZLE_G,
                .b = VK_COMPONENT_SWIZZLE_B,
                .a = VK_COMPONENT_SWIZZLE_A,
            },
        .subresourceRange =
            {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                // mip level count must be 1 for swapchain images
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1,
            },
    };

    for (u64 i = 0; i < _images.size(); ++i) {
        view_create_info.image = _images[i];
        VK_CHECK_RESULT(vkCreateImageView(_device->device(), &view_create_info, _custom_allocator, &_image_views[i]));
    }

    return true;
}

bool VulkanSwapchain::create_vertex_buffer() {
    const VkBufferUsageFlags2CreateInfo staging_usage_flags{
        .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO,
        .pNext = nullptr,
        .usage = VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT,
    };

    const VkBufferUsageFlags2CreateInfo vertex_usage_flags{
        .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO,
        .pNext = nullptr,
        .usage = VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_2_TRANSFER_DST_BIT,
    };
    const VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();
    const VkMemoryPropertyFlags property_flags =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkBuffer staging_buf;
    VkDeviceMemory staging_buf_mem;

    if (!_device->create_buffer(buffer_size,
                                staging_usage_flags,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                &staging_buf,
                                &staging_buf_mem)) {
        CORE_LOG_ERROR("(Vulkan-Swapchain) Failed to create staging buffer.")
        return false;
    }

    if (!_device->create_buffer(buffer_size,
                                vertex_usage_flags,
                                property_flags,
                                &_vertex_buffer,
                                &_vertex_buffer_memory)) {
        CORE_LOG_ERROR("(Vulkan-Swapchain) Failed to create vertex buffer.")
        return false;
    };

    void *data;
    VK_CHECK_RESULT(vkMapMemory(_device->device(), staging_buf_mem, 0, buffer_size, 0, &data));
    std::memcpy(data, &vertices[0], buffer_size);
    vkUnmapMemory(_device->device(), staging_buf_mem);

    _device->copy_buffer(_vertex_buffer, staging_buf, buffer_size);
    vkDestroyBuffer(_device->device(), staging_buf, _custom_allocator);
    vkFreeMemory(_device->device(), staging_buf_mem, _custom_allocator);
    return true;
}

bool VulkanSwapchain::destroy_swapchain() {
    for (VkImageView const &view : _image_views) {
        vkDestroyImageView(_device->device(), view, _custom_allocator);
        _image_views.clear();
    }
    if (_handle != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(_device->device(), _handle, _custom_allocator);
        _handle = VK_NULL_HANDLE;
    }
    return true;
}

bool VulkanSwapchain::destroy_buffers() {
    for (u64 i = 0; i < FRAMES_IN_FLIGHT; ++i) {
        vkFreeMemory(_device->device(), _uniform_buffers_memory[i], _custom_allocator);
        vkDestroyBuffer(_device->device(), _uniform_buffers[i], _custom_allocator);
    }

    vkFreeMemory(_device->device(), _index_buffer_memory, _custom_allocator);
    vkDestroyBuffer(_device->device(), _index_buffer, _custom_allocator);
    vkFreeMemory(_device->device(), _vertex_buffer_memory, _custom_allocator);
    vkDestroyBuffer(_device->device(), _vertex_buffer, _custom_allocator);

    vkDestroyDescriptorPool(_device->device(), _descriptor_pool, _custom_allocator);
    vkDestroyDescriptorSetLayout(_device->device(), _descriptor_set_layout, _custom_allocator);
    return true;
}

bool VulkanSwapchain::acquire_next_image(u64 timeout,
                                         VkSemaphore const &image_available_semaphore,
                                         VkFence const &fence,
                                         u32 *out_index) const {
    VkAcquireNextImageInfoKHR info{
        .sType      = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
        .pNext      = nullptr,
        .swapchain  = _handle,
        .timeout    = timeout,
        .semaphore  = image_available_semaphore,
        .fence      = fence,
        .deviceMask = 1,
    };

    VK_CHECK_RESULT(vkAcquireNextImage2KHR(_device->device(), &info, out_index));
    return true;
}

}  // namespace Pastel::Renderer::Vulkan
