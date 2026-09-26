#pragma once

#include "Rendering/Buffer.hpp"

namespace PicoEngine::Rendering
{

struct MeshVertex
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec4 color;
};

class Mesh
{
  public:
    Mesh(Device& device, std::span<const MeshVertex> vertices, std::span<const uint16_t> indices);
    Mesh(Device& device, std::span<const MeshVertex> vertices, std::span<const uint32_t> indices);
    ~Mesh() = default;

    Mesh(const Mesh&)            = delete;
    Mesh& operator=(const Mesh&) = delete;

    VkBuffer VertexBuffer() const
    {
        return m_vertices->Handle();
    }
    VkBuffer IndexBuffer() const
    {
        return m_indices->Handle();
    }
    uint32_t IndexCount() const
    {
        return m_indexCount;
    }
    VkIndexType IndexType() const
    {
        return m_indexType;
    }

    static VkVertexInputBindingDescription                  BindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> AttributeDescriptions();

    static std::shared_ptr<Mesh> CreateCube(Device& device);

  private:
    Device&                 m_device;
    std::unique_ptr<Buffer> m_vertices;
    std::unique_ptr<Buffer> m_indices;
    uint32_t                m_indexCount = 0;
    VkIndexType             m_indexType  = VK_INDEX_TYPE_UINT16;
};

} // namespace PicoEngine::Rendering
