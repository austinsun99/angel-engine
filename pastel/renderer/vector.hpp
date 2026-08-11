#pragma once

#include <vulkan/vulkan_core.h>
#include <array>
#include <cstddef>
#include <glm/ext/matrix_float4x4.hpp>
namespace Pastel::Renderer::Vulkan {

// @todo: move this to math header
struct Vec2 {
    float x;
    float y;
};

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Vec4 {
    float w;
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

struct UniformBuffer {
    // @todo: replace with own math library mat4
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

}  // namespace Pastel::Renderer::Vulkan
