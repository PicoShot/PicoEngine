#pragma once
#include "Rendering/Buffer.hpp"
#include "Rendering/GraphicsPipeline.hpp"
#include "Rendering/Renderer.hpp"

namespace PicoEngine
{
namespace IO
{
class Vfs;
}
class RotatingCube
{
  public:
    explicit RotatingCube(Rendering::Renderer& renderer, IO::Vfs& vfs);
    ~RotatingCube();
    void Draw(VkCommandBuffer command, float seconds);

  private:
    Rendering::Renderer&                         m_renderer;
    std::unique_ptr<Rendering::Shader>           m_vertexShader;
    std::unique_ptr<Rendering::Shader>           m_fragmentShader;
    std::unique_ptr<Rendering::Buffer>           m_vertices;
    std::unique_ptr<Rendering::Buffer>           m_indices;
    std::unique_ptr<Rendering::GraphicsPipeline> m_pipeline;
    uint64_t                                     m_generation = 0;
};
} // namespace PicoEngine
