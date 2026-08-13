#include <vulkan/vulkan_core.h>
#include "renderer/vulkan_swapchain.h"
#include "renderer/vulkan_utils.h"
#include "vulkan_command_buffer.h"

namespace Pastel::Renderer::Vulkan {

bool VulkanCommandBuffer::create(VkCommandBufferLevel level) {
    const VkCommandBufferAllocateInfo alloc_info{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = _pool,
        .level              = level,
        .commandBufferCount = 1,
    };

    VK_CHECK_RESULT(vkAllocateCommandBuffers(_device, &alloc_info, &_handle));
    return true;
}

bool VulkanCommandBuffer::begin(bool one_time_submit, bool render_pass_continue, bool simultaneous_use) {
    VkCommandBufferUsageFlags flags = 0;
    if (one_time_submit) flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (render_pass_continue) flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    if (simultaneous_use) flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    const VkCommandBufferBeginInfo begin_info{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = nullptr,
        .flags            = flags,
        .pInheritanceInfo = nullptr,
    };
    VK_CHECK_RESULT(vkBeginCommandBuffer(_handle, &begin_info));
    return true;
}

bool VulkanCommandBuffer::end() {
    VK_CHECK_RESULT(vkEndCommandBuffer(_handle));
    return true;
}
bool VulkanCommandBuffer::reset() {
    VK_CHECK_RESULT(vkResetCommandBuffer(_handle, 0));
    return true;
}
bool VulkanCommandBuffer::free() {
    vkFreeCommandBuffers(_device, _pool, 1, &_handle);
    _handle = VK_NULL_HANDLE;
    return true;
}

bool VulkanCommandBuffer::transition_image_layout(VkImage const &image,
                                                  VkImageLayout old_layout,
                                                  VkImageLayout new_layout,
                                                  VkAccessFlags2 src_access_mask,
                                                  VkAccessFlags2 dst_access_mask,
                                                  VkPipelineStageFlags2 src_stage_mask,
                                                  VkPipelineStageFlags dst_stage_mask) {
    VkImageMemoryBarrier2 barrier{
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext               = nullptr,
        .srcStageMask        = src_stage_mask,
        .srcAccessMask       = src_access_mask,
        .dstStageMask        = dst_stage_mask,
        .dstAccessMask       = dst_access_mask,
        .oldLayout           = old_layout,
        .newLayout           = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange{
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        },
    };

    VkDependencyInfo dependency{
        .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext                    = nullptr,
        .dependencyFlags          = {},
        .memoryBarrierCount       = 0,
        .pMemoryBarriers          = nullptr,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers    = nullptr,
        .imageMemoryBarrierCount  = 1,
        .pImageMemoryBarriers     = &barrier,
    };

    vkCmdPipelineBarrier2(_handle, &dependency);
    return true;
}

}  // namespace Pastel::Renderer::Vulkan
