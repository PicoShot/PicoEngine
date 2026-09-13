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
    std::unique_ptr<Rendering::Texture2D>        m_texture;
    VkSampler                                    m_sampler   = VK_NULL_HANDLE;
    VkDescriptorSetLayout                        m_setLayout = VK_NULL_HANDLE;
    VkDescriptorPool                             m_pool      = VK_NULL_HANDLE;
    VkDescriptorSet                              m_set       = VK_NULL_HANDLE;
    std::unique_ptr<Rendering::GraphicsPipeline> m_pipeline;
    uint64_t                                     m_generation = 0;
};
} // namespace PicoEngine
