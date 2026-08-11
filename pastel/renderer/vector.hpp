#pragma once

#include <vulkan/vulkan_core.h>
#include <array>
#include <cstddef>
namespace Pastel::Renderer::Vulkan {

struct Vec2 {
    float x;
    float y;
};

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Vertex {
    Vec2 position;
    Vec3 colour;

    static VkVertexInputBindingDescription get_binding_description() {
        return {
            .binding   = 0,
            .stride    = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
    }

    static std::array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions() {
        return std::array<VkVertexInputAttributeDescription, 2>{
            VkVertexInputAttributeDescription{
                .location = 0,
                .binding  = 0,
                .format   = VK_FORMAT_R32G32_SFLOAT,
                .offset   = offsetof(Vertex, position),
            },
            VkVertexInputAttributeDescription{
                .location = 1,
                .binding  = 0,
                .format   = VK_FORMAT_R32G32B32_SFLOAT,
                .offset   = offsetof(Vertex, colour),
            },
        };
    }
};

}  // namespace Pastel::Renderer::Vulkan
