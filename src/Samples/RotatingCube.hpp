#pragma once
#include "Rendering/Buffer.hpp"
#include "Rendering/GraphicsPipeline.hpp"
#include "Rendering/Renderer.hpp"

namespace PicoEngine
{
class RotatingCube
{
  public:
    explicit RotatingCube(Rendering::Renderer& renderer);
    ~RotatingCube();
    void Draw(VkCommandBuffer command, float seconds);

  private:
    Rendering::Renderer&                         m_renderer;
    Rendering::Shader                            m_vertexShader;
    Rendering::Shader                            m_fragmentShader;
    std::unique_ptr<Rendering::Buffer>           m_vertices;
    std::unique_ptr<Rendering::Buffer>           m_indices;
    std::unique_ptr<Rendering::GraphicsPipeline> m_pipeline;
    uint64_t                                     m_generation = 0;
};
} // namespace PicoEngine
