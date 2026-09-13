#include "GraphicsPipeline.hpp"

namespace PicoEngine::Rendering
{
GraphicsPipeline::GraphicsPipeline(Device& device, const GraphicsPipelineDesc& desc) : m_device(device)
{
    VkPushConstantRange        push{desc.pushConstantStages, 0, desc.pushConstantBytes};
    VkPipelineLayoutCreateInfo layout{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layout.setLayoutCount         = static_cast<uint32_t>(desc.descriptorSets.size());
    layout.pSetLayouts            = desc.descriptorSets.data();
    layout.pushConstantRangeCount = desc.pushConstantBytes ? 1 : 0;
    layout.pPushConstantRanges    = &push;
    Check(vkCreatePipelineLayout(device.Handle(), &layout, nullptr, &m_layout), "Create pipeline layout");
    try
    {
        std::array<VkPipelineShaderStageCreateInfo, 2> stages{};
        for (auto& stage : stages)
        {
            stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stage.pName = "main";
        }
        stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = desc.vertex.Handle();
        stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = desc.fragment.Handle();
        VkSpecializationInfo vertexSpec{};
        if (!desc.vertexSpecEntries.empty())
        {
            vertexSpec.mapEntryCount      = static_cast<uint32_t>(desc.vertexSpecEntries.size());
            vertexSpec.pMapEntries        = desc.vertexSpecEntries.data();
            vertexSpec.dataSize           = desc.vertexSpecData.size();
            vertexSpec.pData              = desc.vertexSpecData.data();
            stages[0].pSpecializationInfo = &vertexSpec;
        }
        VkSpecializationInfo fragmentSpec{};
        if (!desc.fragmentSpecEntries.empty())
        {
            fragmentSpec.mapEntryCount    = static_cast<uint32_t>(desc.fragmentSpecEntries.size());
            fragmentSpec.pMapEntries      = desc.fragmentSpecEntries.data();
            fragmentSpec.dataSize         = desc.fragmentSpecData.size();
            fragmentSpec.pData            = desc.fragmentSpecData.data();
            stages[1].pSpecializationInfo = &fragmentSpec;
        }
        VkPipelineVertexInputStateCreateInfo vertex{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        vertex.vertexBindingDescriptionCount   = static_cast<uint32_t>(desc.bindings.size());
        vertex.pVertexBindingDescriptions      = desc.bindings.data();
        vertex.vertexAttributeDescriptionCount = static_cast<uint32_t>(desc.attributes.size());
        vertex.pVertexAttributeDescriptions    = desc.attributes.data();
        VkPipelineInputAssemblyStateCreateInfo assembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkPipelineViewportStateCreateInfo viewport{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewport.viewportCount = 1;
        viewport.scissorCount  = 1;
        VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        raster.polygonMode = VK_POLYGON_MODE_FILL;
        raster.cullMode    = VK_CULL_MODE_BACK_BIT;
        raster.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        raster.lineWidth   = 1.0f;
        VkPipelineMultisampleStateCreateInfo multisample{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        VkPipelineDepthStencilStateCreateInfo depth{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        depth.depthTestEnable  = desc.depthTest;
        depth.depthWriteEnable = desc.depthTest;
        depth.depthCompareOp   = VK_COMPARE_OP_LESS;
        VkPipelineColorBlendAttachmentState attachment{};
        attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        blend.attachmentCount = 1;
        blend.pAttachments    = &attachment;
        std::array<VkDynamicState, 2>    states{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamic{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamic.dynamicStateCount = static_cast<uint32_t>(states.size());
        dynamic.pDynamicStates    = states.data();
        VkGraphicsPipelineCreateInfo info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        info.stageCount          = static_cast<uint32_t>(stages.size());
        info.pStages             = stages.data();
        info.pVertexInputState   = &vertex;
        info.pInputAssemblyState = &assembly;
        info.pViewportState      = &viewport;
        info.pRasterizationState = &raster;
        info.pMultisampleState   = &multisample;
        info.pDepthStencilState  = &depth;
        info.pColorBlendState    = &blend;
        info.pDynamicState       = &dynamic;
        info.layout              = m_layout;
        info.renderPass          = desc.renderPass;
        Check(vkCreateGraphicsPipelines(device.Handle(), VK_NULL_HANDLE, 1, &info, nullptr, &m_pipeline), "Create graphics pipeline");
    }
    catch (const std::exception& exception)
    {
        if (m_pipeline) vkDestroyPipeline(device.Handle(), m_pipeline, nullptr);
        vkDestroyPipelineLayout(device.Handle(), m_layout, nullptr);
        PICO_ASSERT_FAIL("Pipeline creation failed: {}", exception.what());
    }
}
GraphicsPipeline::~GraphicsPipeline()
{
    vkDestroyPipeline(m_device.Handle(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device.Handle(), m_layout, nullptr);
}
} // namespace PicoEngine::Rendering
