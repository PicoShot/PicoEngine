#include "Mesh.hpp"

namespace PicoEngine::Rendering
{

Mesh::Mesh(Device& device, std::span<const MeshVertex> vertices, std::span<const uint16_t> indices)
    : m_device(device), m_indexCount(static_cast<uint32_t>(indices.size())), m_indexType(VK_INDEX_TYPE_UINT16)
{
    PICO_ASSERT(!vertices.empty() && !indices.empty(), "Mesh needs vertices and indices");
    m_vertices = Buffer::Upload(device, std::as_bytes(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    m_indices  = Buffer::Upload(device, std::as_bytes(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

Mesh::Mesh(Device& device, std::span<const MeshVertex> vertices, std::span<const uint32_t> indices)
    : m_device(device), m_indexCount(static_cast<uint32_t>(indices.size())), m_indexType(VK_INDEX_TYPE_UINT32)
{
    PICO_ASSERT(!vertices.empty() && !indices.empty(), "Mesh needs vertices and indices");
    m_vertices = Buffer::Upload(device, std::as_bytes(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    m_indices  = Buffer::Upload(device, std::as_bytes(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

VkVertexInputBindingDescription Mesh::BindingDescription()
{
    return VkVertexInputBindingDescription{0, sizeof(MeshVertex), VK_VERTEX_INPUT_RATE_VERTEX};
}

std::array<VkVertexInputAttributeDescription, 3> Mesh::AttributeDescriptions()
{
    return std::array<VkVertexInputAttributeDescription, 3>{{{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(MeshVertex, position)},
                                                             {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(MeshVertex, uv)},
                                                             {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MeshVertex, color)}}};
}

std::shared_ptr<Mesh> Mesh::CreateCube(Device& device)
{
    const std::array<MeshVertex, 24> vertices{{
        {{-1, -1, -1}, {0, 0}, {0.2f, 0.4f, 1, 1}}, // -Z
        {{1, -1, -1}, {1, 0}, {1, 0.3f, 0.2f, 1}},
        {{1, 1, -1}, {1, 1}, {1, 0.8f, 0.2f, 1}},
        {{-1, 1, -1}, {0, 1}, {0.2f, 1, 0.5f, 1}},
        {{-1, -1, 1}, {0, 0}, {0.6f, 0.2f, 1, 1}}, // +Z
        {{1, -1, 1}, {1, 0}, {1, 0.3f, 0.6f, 1}},
        {{1, 1, 1}, {1, 1}, {0.3f, 0.8f, 1, 1}},
        {{-1, 1, 1}, {0, 1}, {0.3f, 1, 0.7f, 1}},
        {{-1, -1, -1}, {0, 0}, {0.2f, 0.4f, 1, 1}}, // -X
        {{-1, -1, 1}, {1, 0}, {0.6f, 0.2f, 1, 1}},
        {{-1, 1, 1}, {1, 1}, {0.3f, 1, 0.7f, 1}},
        {{-1, 1, -1}, {0, 1}, {0.2f, 1, 0.5f, 1}},
        {{1, -1, -1}, {0, 0}, {1, 0.3f, 0.2f, 1}}, // +X
        {{1, -1, 1}, {1, 0}, {1, 0.3f, 0.6f, 1}},
        {{1, 1, 1}, {1, 1}, {0.3f, 0.8f, 1, 1}},
        {{1, 1, -1}, {0, 1}, {1, 0.8f, 0.2f, 1}},
        {{-1, -1, -1}, {0, 0}, {0.2f, 0.4f, 1, 1}}, // -Y
        {{1, -1, -1}, {1, 0}, {1, 0.3f, 0.2f, 1}},
        {{1, -1, 1}, {1, 1}, {1, 0.3f, 0.6f, 1}},
        {{-1, -1, 1}, {0, 1}, {0.6f, 0.2f, 1, 1}},
        {{-1, 1, -1}, {0, 0}, {0.2f, 1, 0.5f, 1}}, // +Y
        {{1, 1, -1}, {1, 0}, {1, 0.8f, 0.2f, 1}},
        {{1, 1, 1}, {1, 1}, {0.3f, 0.8f, 1, 1}},
        {{-1, 1, 1}, {0, 1}, {0.3f, 1, 0.7f, 1}},
    }};
    const std::array<uint16_t, 36>   indices{{0, 2, 1, 0, 3, 2, 4, 5, 6, 4, 6, 7,
                                              8, 9, 10, 8, 10, 11, 12, 15, 14, 12, 14, 13,
                                              16, 17, 18, 16, 18, 19, 20, 23, 22, 20, 22, 21}};
    return std::make_shared<Mesh>(device, std::span(vertices), std::span(indices));
}

} // namespace PicoEngine::Rendering
