#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include "renderer/vulkan_utils.h"
namespace Pastel::Renderer::Vulkan {

template <u32 frames_in_flight>
struct VulkanSyncObject {
   private:
    VkAllocationCallbacks *_custom_allocator;
    VkDevice _device;

    std::array<VkFence, frames_in_flight> _fences;

    std::array<VkSemaphore, frames_in_flight> _image_available_sems;
    std::vector<VkSemaphore> _queue_complete_sems;

   public:
    VulkanSyncObject() = default;

    bool create(u32 image_count,
                bool fences_signaled,
                VkDevice const &device,
                VkAllocationCallbacks *const &custom_allocator) {
        _custom_allocator              = custom_allocator;
        _device                        = device;
        const VkFenceCreateFlags flags = fences_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

        VkFenceCreateInfo fence_create_info{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
        };
        for (VkFence &fence : _fences)
            VK_CHECK_RESULT(vkCreateFence(_device, &fence_create_info, _custom_allocator, &fence));

        const VkSemaphoreCreateInfo sem_create_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };

        _queue_complete_sems.resize(image_count);
        for (VkSemaphore &sem : _queue_complete_sems)
            VK_CHECK_RESULT(vkCreateSemaphore(_device, &sem_create_info, _custom_allocator, &sem));

        for (VkSemaphore &sem : _image_available_sems)
            VK_CHECK_RESULT(vkCreateSemaphore(_device, &sem_create_info, _custom_allocator, &sem));
        return true;
    }

    bool wait_fence(u32 index, u64 timeout) const {
        VK_CHECK_RESULT(vkWaitForFences(_device, 1, &_fences[index], VK_TRUE, timeout));
        return true;
    }

    bool reset_fence(u32 index) const {
        VK_CHECK_RESULT(vkResetFences(_device, 1, &_fences[index]));
        return true;
    }

    bool destroy() {
        for (VkFence &fence : _fences) vkDestroyFence(_device, fence, _custom_allocator);

        for (VkSemaphore &sem : _queue_complete_sems) vkDestroySemaphore(_device, sem, _custom_allocator);
        for (VkSemaphore &sem : _image_available_sems) vkDestroySemaphore(_device, sem, _custom_allocator);
        return true;
    }

    VkSemaphore const &image_available_sem(u64 index) const {
        return _image_available_sems[index];
    }

    VkSemaphore const &queue_complete_sem(u64 index) const {
        return _queue_complete_sems[index];
    }

    VkFence const &fence(u64 index) const {
        return _fences[index];
    }
};

}  // namespace Pastel::Renderer::Vulkan
