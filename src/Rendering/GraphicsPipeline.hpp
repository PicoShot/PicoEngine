#pragma once
#include "Shader.hpp"

namespace PicoEngine::Rendering
{
struct GraphicsPipelineDesc
{
    VkRenderPass                                       renderPass = VK_NULL_HANDLE;
    const Shader&                                      vertex;
    const Shader&                                      fragment;
    std::span<const VkVertexInputBindingDescription>   bindings;
    std::span<const VkVertexInputAttributeDescription> attributes;
    uint32_t                                           pushConstantBytes  = 0;
    VkShaderStageFlags                                 pushConstantStages = VK_SHADER_STAGE_VERTEX_BIT;

    std::span<const VkSpecializationMapEntry> vertexSpecEntries;
    std::span<const std::byte>                vertexSpecData;
    std::span<const VkSpecializationMapEntry> fragmentSpecEntries;
    std::span<const std::byte>                fragmentSpecData;
    std::span<const VkDescriptorSetLayout>    descriptorSets;
    bool                                      depthTest = true;
};

class GraphicsPipeline
{
  public:
    GraphicsPipeline(Device& device, const GraphicsPipelineDesc& desc);
    ~GraphicsPipeline();
    GraphicsPipeline(const GraphicsPipeline&)            = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
    VkPipeline        Handle() const
    {
        return m_pipeline;
    }
    VkPipelineLayout Layout() const
    {
        return m_layout;
    }

  private:
    Device&          m_device;
    VkPipelineLayout m_layout   = VK_NULL_HANDLE;
    VkPipeline       m_pipeline = VK_NULL_HANDLE;
};
} // namespace PicoEngine::Rendering
