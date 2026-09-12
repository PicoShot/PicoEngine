#include "RotatingCube.hpp"

namespace PicoEngine
{
namespace
{
struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
};
std::filesystem::path ShaderPath(const char* name)
{
    const char* base = SDL_GetBasePath();
    if (!base) throw std::runtime_error(SDL_GetError());
    return std::filesystem::path(base) / "shaders" / name;
}
} // namespace
RotatingCube::RotatingCube(Rendering::Renderer& renderer)
    : m_renderer(renderer), m_vertexShader(renderer.GetDevice(), ShaderPath("cube.vert.spv")),
      m_fragmentShader(renderer.GetDevice(), ShaderPath("cube.frag.spv"))
{
    const std::array<Vertex, 8>    vertices{{{{-1, -1, -1}, {0.2f, 0.4f, 1}}, {{1, -1, -1}, {1, 0.3f, 0.2f}}, {{1, 1, -1}, {1, 0.8f, 0.2f}}, {{-1, 1, -1}, {0.2f, 1, 0.5f}}, {{-1, -1, 1}, {0.6f, 0.2f, 1}}, {{1, -1, 1}, {1, 0.3f, 0.6f}}, {{1, 1, 1}, {0.3f, 0.8f, 1}}, {{-1, 1, 1}, {0.3f, 1, 0.7f}}}};
    const std::array<uint16_t, 36> indices{{0, 2, 1, 0, 3, 2, 4, 5, 6, 4, 6, 7,
                                            0, 4, 7, 0, 7, 3, 1, 2, 6, 1, 6, 5,
                                            3, 7, 6, 3, 6, 2, 0, 1, 5, 0, 5, 4}};
    m_vertices = Rendering::Buffer::Upload(renderer.GetDevice(), std::as_bytes(std::span(vertices)), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    m_indices  = Rendering::Buffer::Upload(renderer.GetDevice(), std::as_bytes(std::span(indices)), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}
RotatingCube::~RotatingCube()
{
    // Sample-owned resources may still be referenced by the last two frames.
    vkDeviceWaitIdle(m_renderer.GetDevice().Handle());
}
void RotatingCube::Draw(VkCommandBuffer command, float seconds)
{
    if (m_generation != m_renderer.Generation())
    {
        VkVertexInputBindingDescription                  binding{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
        std::array<VkVertexInputAttributeDescription, 2> attributes{{{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
                                                                     {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)}}};
        Rendering::GraphicsPipelineDesc                  desc{m_renderer.RenderPass(), m_vertexShader, m_fragmentShader,
                                                              std::span(&binding, 1), attributes, sizeof(glm::mat4)};
        m_pipeline   = std::make_unique<Rendering::GraphicsPipeline>(m_renderer.GetDevice(), desc);
        m_generation = m_renderer.Generation();
    }
    auto extent     = m_renderer.Extent();
    auto projection = glm::perspectiveRH_ZO(glm::radians(50.0f), float(extent.width) / float(extent.height), 0.1f, 100.0f);
    projection[1][1] *= -1.0f;
    auto      view  = glm::lookAtRH(glm::vec3(0, 1.5f, 6), glm::vec3(0), glm::vec3(0, 1, 0));
    auto      model = glm::rotate(glm::mat4(1), seconds * 0.8f, glm::normalize(glm::vec3(0.4f, 1, 0.2f)));
    glm::mat4 mvp   = projection * view * model;
    vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->Handle());
    VkBuffer     vertex = m_vertices->Handle();
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(command, 0, 1, &vertex, &offset);
    vkCmdBindIndexBuffer(command, m_indices->Handle(), 0, VK_INDEX_TYPE_UINT16);
    vkCmdPushConstants(command, m_pipeline->Layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvp), &mvp);
    vkCmdDrawIndexed(command, 36, 1, 0, 0, 0);
}
} // namespace PicoEngine
