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
    uint32_t                                           pushConstantBytes = 0;
    bool                                               depthTest         = true;
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
