#include "vulkan_graphics_pipeline.h"
#include "renderer/vector.hpp"
#include "vulkan_utils.h"
#include <vulkan/vulkan_core.h>
#include "core/io/filesystem.h"

namespace angel::renderer::vulkan {

bool GraphicsPipeline::create(VkDescriptorSetLayout descriptor_set_layout) {
    const u32 dynamic_state_count                      = 2;
    VkDynamicState dynamic_states[dynamic_state_count] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext             = nullptr,
        .flags             = 0,
        .dynamicStateCount = dynamic_state_count,
        .pDynamicStates    = dynamic_states,
    };

    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.scissorCount  = 1;

    VkVertexInputBindingDescription binding_description                     = Vertex::get_binding_description();
    std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions = Vertex::get_attribute_descriptions();
    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext                           = nullptr,
        .flags                           = 0,
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &binding_description,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::get_attribute_descriptions().size()),
        .pVertexAttributeDescriptions    = &attribute_descriptions[0],
    };
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
    input_assembly_state_create_info.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
    rasterization_state_create_info.sType           = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_create_info.depthBiasEnable = VK_FALSE;
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterization_state_create_info.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterization_state_create_info.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_state_create_info.depthBiasEnable         = VK_FALSE;
    rasterization_state_create_info.lineWidth               = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
    multisample_state_create_info.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state_create_info.sampleShadingEnable  = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
    color_blend_attachment_state.blendEnable = VK_FALSE;
    color_blend_attachment_state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
    color_blend_state_create_info.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.logicOpEnable   = VK_FALSE;
    color_blend_state_create_info.logicOp         = VK_LOGIC_OP_COPY;
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments    = &color_blend_attachment_state;

    VkPipelineLayoutCreateInfo layout_create_info{};
    layout_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_create_info.setLayoutCount         = 1;
    layout_create_info.pSetLayouts            = &descriptor_set_layout;
    layout_create_info.pushConstantRangeCount = 0;

    VK_CHECK_RESULT(vkCreatePipelineLayout(_device->device(), &layout_create_info, _custom_allocator, &_layout));

    VkShaderModule vert_shader_mod{};
    VkShaderModule frag_shader_mod{};
    create_shader_module("build/assets/shaders/triangle.vert.spv", &vert_shader_mod);
    create_shader_module("build/assets/shaders/triangle.frag.spv", &frag_shader_mod);
    _shader_modules.push_back(vert_shader_mod);
    _shader_modules.push_back(frag_shader_mod);

    VkPipelineShaderStageCreateInfo vert_shader_stage_create_info{};
    vert_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_create_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_create_info.module = vert_shader_mod;
    vert_shader_stage_create_info.pName  = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_create_info{};
    frag_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_create_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_create_info.module = frag_shader_mod;
    frag_shader_stage_create_info.pName  = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {
        vert_shader_stage_create_info,
        frag_shader_stage_create_info,
    };

    VkFormat swapchain_format;
    _swapchain->selected_format(swapchain_format);

    VkPipelineRenderingCreateInfo rendering_create_info{};
    rendering_create_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    rendering_create_info.colorAttachmentCount    = 1;
    rendering_create_info.pColorAttachmentFormats = &swapchain_format;

    VkGraphicsPipelineCreateInfo pipeline_create_info{
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &rendering_create_info,
        .flags               = 0,
        .stageCount          = 2,
        .pStages             = shader_stages,
        .pVertexInputState   = &vertex_input_state_create_info,
        .pInputAssemblyState = &input_assembly_state_create_info,
        .pTessellationState  = nullptr,
        .pViewportState      = &viewport_state_create_info,
        .pRasterizationState = &rasterization_state_create_info,
        .pMultisampleState   = &multisample_state_create_info,
        .pDepthStencilState  = nullptr,
        .pColorBlendState    = &color_blend_state_create_info,
        .pDynamicState       = &dynamic_state_create_info,
        .layout              = _layout,
        .renderPass          = nullptr,
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
        .basePipelineIndex   = 0,
    };

    VK_CHECK_RESULT(
        vkCreateGraphicsPipelines(_device->device(), nullptr, 1, &pipeline_create_info, _custom_allocator, &_handle));
    return true;
}

bool GraphicsPipeline::destroy() {
    for (VkShaderModule const &_module : _shader_modules) {
        vkDestroyShaderModule(_device->device(), _module, _custom_allocator);
    }

    vkDestroyPipelineLayout(_device->device(), _layout, _custom_allocator);
    vkDestroyPipeline(_device->device(), _handle, _custom_allocator);
    return true;
}

bool GraphicsPipeline::create_shader_module(const char *file_name, VkShaderModule *out_module) {
    const std::vector<char> code = io::read_file(file_name);
    const uint32_t *code_data    = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModuleCreateInfo create_info{
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = 0,
        .codeSize = code.size() * sizeof(char),
        .pCode    = code_data,

    };

    VK_CHECK_RESULT(vkCreateShaderModule(_device->device(), &create_info, _custom_allocator, out_module));
    return true;
}

}  // namespace angel::renderer::vulkan
