#pragma once

#include <vulkan/vulkan_core.h>
#include "renderer/vulkan_utils.h"
namespace Pastel::Renderer::Vulkan {

struct VulkanSyncObject {
   private:
    VkAllocationCallbacks *const& _custom_allocator;
    VkDevice const &_device;
    VkFence _fence;

    VkSemaphore _queue_complete_sem;
    VkSemaphore _image_available_sem;
   public:
    VulkanSyncObject(VkDevice const &device, VkAllocationCallbacks* const& custom_allocator) : _custom_allocator(custom_allocator), _device(device) {
    }


    bool create(bool signaled) {
        const VkFenceCreateFlags flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
        VkFenceCreateInfo fence_create_info{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
        };
        VK_CHECK_RESULT(vkCreateFence(_device, &fence_create_info, _custom_allocator, &_fence));

        const VkSemaphoreCreateInfo sem_create_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };
        VK_CHECK_RESULT(vkCreateSemaphore(_device, &sem_create_info, _custom_allocator, &_queue_complete_sem));
        VK_CHECK_RESULT(vkCreateSemaphore(_device, &sem_create_info, _custom_allocator, &_image_available_sem));
        return true;
    }

    bool wait_fence(u64 timeout) const {
        VK_CHECK_RESULT(vkWaitForFences(_device, 1, &_fence, VK_TRUE, timeout));
        return true;
    }

    bool reset_fence() const {
        VK_CHECK_RESULT(vkResetFences(_device, 1, &_fence));
        return true;
    }

    bool destroy() {
        vkDestroyFence(_device, _fence, _custom_allocator);
        _fence = VK_NULL_HANDLE;

        vkDestroySemaphore(_device, _queue_complete_sem, _custom_allocator);
        vkDestroySemaphore(_device, _image_available_sem, _custom_allocator);
        _queue_complete_sem = VK_NULL_HANDLE;
        _image_available_sem = VK_NULL_HANDLE;

        return true;
    }

    VkSemaphore const& image_available_sem() const {
        return _image_available_sem;
    }

    VkSemaphore const& queue_complete_sem() const {
        return _queue_complete_sem;
    }

    VkFence const& fence() const {
        return _fence;
    }
};

}  // namespace Pastel::Renderer::Vulkan
