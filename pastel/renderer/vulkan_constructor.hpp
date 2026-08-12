#pragma once

#include <vulkan/vulkan_core.h>
#include <cstring>
#include <vector>
#include "pastel_types.h"
#include "renderer/vulkan_device.h"
#include "renderer/vulkan_renderer.h"
#include "renderer/vulkan_utils.h"
namespace Pastel::Renderer::Vulkan {

static inline bool create_buffer(const VulkanDevice *device,
                                 u32 queue_family_index,
                                 VkDeviceSize size,
                                 VkBufferUsageFlags2CreateInfo const &usage_flags,
                                 VkMemoryPropertyFlags properties,
                                 VkBuffer *const &out_buffer,
                                 VkDeviceMemory *const &out_memory,
                                 VkAllocationCallbacks *custom_allocator = nullptr) {
    VkBufferCreateInfo buffer_create_info{
        .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext                 = &usage_flags,
        .flags                 = 0,
        .size                  = size,
        .usage                 = 0,
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices   = &queue_family_index,
    };

    VK_CHECK_RESULT(vkCreateBuffer(device->device(), &buffer_create_info, custom_allocator, out_buffer));

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device->device(), *out_buffer, &memory_requirements);

    u32 memory_index = 0;
    if (!device->find_suitable_memory_type(memory_requirements.memoryTypeBits, properties, &memory_index)) {
        CORE_LOG_WARN("(Vulkan-Swapchain) Could not find suitable memory type.")
        return false;
    }

    VkMemoryAllocateInfo allocate_info{
        .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext           = nullptr,
        .allocationSize  = memory_requirements.size,
        .memoryTypeIndex = memory_index,
    };
    VK_CHECK_RESULT(vkAllocateMemory(device->device(), &allocate_info, custom_allocator, out_memory));
    VK_CHECK_RESULT(vkBindBufferMemory(device->device(), *out_buffer, *out_memory, 0));

    return true;
}

static inline bool create_buffer(const VulkanDevice *device,
                                 u32 queue_family_index,
                                 VkDeviceSize size,
                                 VkBufferUsageFlags2 const &usage_flags,
                                 VkMemoryPropertyFlags properties,
                                 VkBuffer *const &out_buffer,
                                 VkDeviceMemory *const &out_memory,
                                 VkAllocationCallbacks *custom_allocator = nullptr) {
    VkBufferUsageFlags2CreateInfo flags_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO,
        .pNext = nullptr,
        .usage = usage_flags,
    };
    return create_buffer(device,
                         queue_family_index,
                         size,
                         flags_create_info,
                         properties,
                         out_buffer,
                         out_memory,
                         custom_allocator);
}

static inline bool create_command_pool(VkDevice const &device,
                                       u32 queue_family_index,
                                       VkCommandPoolCreateFlags const create_flags,
                                       VkCommandPool *out_pool,
                                       VkAllocationCallbacks *custom_allocator = nullptr,
                                       const void *extension                   = nullptr) {
    const VkCommandPoolCreateInfo create_info{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = extension,
        .flags            = create_flags,
        .queueFamilyIndex = queue_family_index,
    };

    VK_CHECK_RESULT(vkCreateCommandPool(device, &create_info, custom_allocator, out_pool));
    return true;
}

static inline bool create_staging_buffer_and_stage(VulkanDevice *_device,
                                                   u32 size,
                                                   const void *data_to_stage,
                                                   VkBuffer &buffer_to_stage,
                                                   VkAllocationCallbacks *custom_allocator = nullptr) {
    const VkBufferUsageFlags2CreateInfo staging_usage_flags{
        .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO,
        .pNext = nullptr,
        .usage = VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT,
    };

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_mem;
    if (!create_buffer(_device,
                       _device->device_properties().graphics_queue_index,
                       size,
                       staging_usage_flags,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       &staging_buffer,
                       &staging_buffer_mem,
                       custom_allocator)) {
        return false;
    }

    void *data;
    VK_CHECK_RESULT(vkMapMemory(_device->device(), staging_buffer_mem, 0, size, 0, &data));
    std::memcpy(data, data_to_stage, size);
    vkUnmapMemory(_device->device(), staging_buffer_mem);

    _device->copy_buffer(buffer_to_stage, staging_buffer, size);
    vkDestroyBuffer(_device->device(), staging_buffer, custom_allocator);
    vkFreeMemory(_device->device(), staging_buffer_mem, custom_allocator);
    return true;
};

static inline bool create_descriptor_layout_and_pool(VkDevice device,
                                                     VkDescriptorType descriptor_type,
                                                     VkShaderStageFlags shader_stage_flag,
                                                     u32 descriptor_count,
                                                     VkDescriptorSetLayout *out_layout,
                                                     VkDescriptorPool *out_pool,
                                                     VkAllocationCallbacks *custom_allocator = nullptr) {
    VkDescriptorSetLayoutBinding layout_binding{
        .binding            = 0,
        .descriptorType     = descriptor_type,
        .descriptorCount    = 1,
        .stageFlags         = shader_stage_flag,
        .pImmutableSamplers = nullptr,
    };

    VkDescriptorSetLayoutCreateInfo layout_create_info{
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = nullptr,
        .flags        = 0,
        .bindingCount = 1,
        .pBindings    = &layout_binding,
    };
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layout_create_info, custom_allocator, out_layout));

    VkDescriptorPoolSize pool_size{
        .type            = descriptor_type,
        .descriptorCount = descriptor_count,
    };

    VkDescriptorPoolCreateInfo pool_create_info{
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = 0,
        .maxSets       = descriptor_count,
        .poolSizeCount = 1,
        .pPoolSizes    = &pool_size,
    };

    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_create_info, custom_allocator, out_pool));

    return true;
}

static inline bool allocate_descriptor_sets(VkDevice const &device,
                                            VkDescriptorSetLayout const &descriptor_set_layout,
                                            VkDescriptorPool const &descriptor_pool,
                                            std::vector<VkDescriptorSet> &out_descriptor_sets) {
    std::vector<VkDescriptorSetLayout> layouts(FRAMES_IN_FLIGHT, descriptor_set_layout);
    VkDescriptorSetAllocateInfo set_allocate_info{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = descriptor_pool,
        .descriptorSetCount = FRAMES_IN_FLIGHT,
        .pSetLayouts        = &layouts[0],
    };

    out_descriptor_sets.resize(FRAMES_IN_FLIGHT);
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &set_allocate_info, out_descriptor_sets.data()));
    return true;
}

}  // namespace Pastel::Renderer::Vulkan
