#pragma once

#include "Rendering/GraphicsPipeline.hpp"
#include "Rendering/Texture2D.hpp"

namespace PicoEngine
{
namespace IO
{
class Vfs;
}
} // namespace PicoEngine

namespace PicoEngine::Rendering
{

class Mesh;
class Renderer;

class Material
{
  public:
    static std::shared_ptr<Material> CreateUnlit(Device& device, IO::Vfs& vfs,
                                                 const std::string& texturePath = "");
    ~Material();

    Material(const Material&)            = delete;
    Material& operator=(const Material&) = delete;

    const glm::vec4& GetTint() const
    {
        return m_tint;
    }
    void SetTint(const glm::vec4& tint)
    {
        m_tint = tint;
    }

    void Draw(VkCommandBuffer command, Renderer& renderer, const Mesh& mesh, const glm::mat4& mvp);

  private:
    Material(Device& device, std::unique_ptr<Shader> vertexShader, std::unique_ptr<Shader> fragmentShader,
             std::unique_ptr<Texture2D> texture);

    void EnsurePipeline(Renderer& renderer);

    Device&                           m_device;
    std::unique_ptr<Shader>           m_vertexShader;
    std::unique_ptr<Shader>           m_fragmentShader;
    std::unique_ptr<Texture2D>        m_texture;
    VkSampler                         m_sampler   = VK_NULL_HANDLE;
    VkDescriptorSetLayout             m_setLayout = VK_NULL_HANDLE;
    VkDescriptorPool                  m_pool      = VK_NULL_HANDLE;
    VkDescriptorSet                   m_set       = VK_NULL_HANDLE;
    glm::vec4                         m_tint      = glm::vec4(1.0f);
    glm::vec4                         m_mapST     = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
    std::unique_ptr<GraphicsPipeline> m_pipeline;
    uint64_t                          m_generation = 0;
};

} // namespace PicoEngine::Rendering
