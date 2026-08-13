#pragma once

#include <vulkan/vulkan_core.h>
namespace angel::renderer::vulkan {

// represents 1 command buffer
class VulkanCommandBuffer {
   private:
    VkDevice const &_device;
    VkCommandPool const &_pool;
    VkAllocationCallbacks *const &_custom_allocator;
    VkCommandBuffer _handle = VK_NULL_HANDLE;

   public:
    VulkanCommandBuffer(VkCommandPool const &pool,
                        VkDevice const &device,
                        VkAllocationCallbacks *const &custom_allocator)
        : _device(device), _pool(pool), _custom_allocator(custom_allocator) {
    }

    ~VulkanCommandBuffer() = default;

    bool create(VkCommandBufferLevel level);
    bool free();

    // One time submit indicates the buffer will be submitted once, and reset and recorded again for each submission
    //
    // Render pass continue is only relevant to secondary command buffers. Indicates the secondary buffer is inside the
    // render pass.
    //
    // Simultaneous use indicates the buffer can be resubmitted to the same queue family in the pending
    // state, and can be submitted to multiple primary command buffers.
    bool begin(bool one_time_submit, bool render_pass_continue, bool simultaneous_use);
    bool end();
    bool reset();
    bool transition_image_layout(VkImage const &image,
                                 VkImageLayout old_layout,
                                 VkImageLayout new_layout,
                                 VkAccessFlags2 src_access_mask,
                                 VkAccessFlags2 dst_access_mask,
                                 VkPipelineStageFlags2 src_stage_mask,
                                 VkPipelineStageFlags dst_stage_mask);

    VkCommandBuffer const &handle() const {
        return _handle;
    }
};
}  // namespace angel::renderer::vulkan
