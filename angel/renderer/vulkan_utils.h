#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <memory>

#include "core/logging/logger.h"
#include "defines.h"
namespace angel::renderer::vulkan {

ANGEL_FORCE_INLINE static bool vulkan_check_result(VkResult result, const char *func, const char *file, int line) {
    if (result != VK_SUCCESS) {
        AL_CORE_ERROR("(%s at line %d) [Vulkan Result Error] from %s", file, line, func)
        return false;
    }
    return true;
}

ANGEL_FORCE_INLINE static bool vkb_to_b(VkBool32 vkb) {
    return vkb == VK_TRUE ? true : false;
}

ANGEL_FORCE_INLINE static VkBool32 b_to_vkb(bool b) {
    return b ? VK_TRUE : VK_FALSE;
}

#define VK_CHECK_RESULT(result)                                                                        \
    do {                                                                                               \
        if (!::angel::renderer::vulkan::vulkan_check_result(result, "##result", __FILE__, __LINE__)) { \
            return false;                                                                              \
        }                                                                                              \
    } while (false)

template <typename T>
T *get_vulkan_FPN(VkInstance const &instance, const char *fpn_name) {
    T pfn = std::reinterpret_pointer_cast<T>(vkGetInstanceProcAddr(instance, fpn_name));
    if (pfn == nullptr) {
        AL_CORE_ERROR("(Vulkan Utils) Could not obtain pointer to function %s", fpn_name)
        return nullptr;
    }
    return pfn;
}
}  // namespace angel::renderer::vulkan
