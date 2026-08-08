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
}  // namespace Pastel::Renderer::Vulkan
