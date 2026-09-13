#include "RotatingCube.hpp"
#include "IO/Vfs.hpp"
#include "Rendering/ShaderCompiler.hpp"

namespace PicoEngine
{
namespace
{
struct Vertex
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec4 color;
};
struct UnlitPush
{
    glm::mat4 mvp;
    glm::vec4 tint;
    glm::vec4 mapST;
};
} // namespace
RotatingCube::RotatingCube(Rendering::Renderer& renderer, IO::Vfs& vfs) : m_renderer(renderer)
{
    Rendering::ShaderCompiler compiler(vfs);
    compiler.AddIncludePrefix("shaders/commons");
    auto vertexSpv   = compiler.CompileFile("shaders/unlit/unlit.vert");
    auto fragmentSpv = compiler.CompileFile("shaders/unlit/unlit.frag");

    m_vertexShader   = std::make_unique<Rendering::Shader>(renderer.GetDevice(), vertexSpv.spirv, "shaders/unlit/unlit.vert");
    m_fragmentShader = std::make_unique<Rendering::Shader>(renderer.GetDevice(), fragmentSpv.spirv, "shaders/unlit/unlit.frag");

    const std::array<Vertex, 24> vertices{{
        // -Z
        {{-1, -1, -1}, {0, 0}, {0.2f, 0.4f, 1, 1}},
        {{1, -1, -1}, {1, 0}, {1, 0.3f, 0.2f, 1}},
        {{1, 1, -1}, {1, 1}, {1, 0.8f, 0.2f, 1}},
        {{-1, 1, -1}, {0, 1}, {0.2f, 1, 0.5f, 1}},
        // +Z
        {{-1, -1, 1}, {0, 0}, {0.6f, 0.2f, 1, 1}},
        {{1, -1, 1}, {1, 0}, {1, 0.3f, 0.6f, 1}},
        {{1, 1, 1}, {1, 1}, {0.3f, 0.8f, 1, 1}},
        {{-1, 1, 1}, {0, 1}, {0.3f, 1, 0.7f, 1}},
        // -X
        {{-1, -1, -1}, {0, 0}, {0.2f, 0.4f, 1, 1}},
        {{-1, -1, 1}, {1, 0}, {0.6f, 0.2f, 1, 1}},
        {{-1, 1, 1}, {1, 1}, {0.3f, 1, 0.7f, 1}},
        {{-1, 1, -1}, {0, 1}, {0.2f, 1, 0.5f, 1}},
        // +X
        {{1, -1, -1}, {0, 0}, {1, 0.3f, 0.2f, 1}},
        {{1, -1, 1}, {1, 0}, {1, 0.3f, 0.6f, 1}},
        {{1, 1, 1}, {1, 1}, {0.3f, 0.8f, 1, 1}},
        {{1, 1, -1}, {0, 1}, {1, 0.8f, 0.2f, 1}},
        // -Y
        {{-1, -1, -1}, {0, 0}, {0.2f, 0.4f, 1, 1}},
        {{1, -1, -1}, {1, 0}, {1, 0.3f, 0.2f, 1}},
        {{1, -1, 1}, {1, 1}, {1, 0.3f, 0.6f, 1}},
        {{-1, -1, 1}, {0, 1}, {0.6f, 0.2f, 1, 1}},
        // +Y
        {{-1, 1, -1}, {0, 0}, {0.2f, 1, 0.5f, 1}},
        {{1, 1, -1}, {1, 0}, {1, 0.8f, 0.2f, 1}},
        {{1, 1, 1}, {1, 1}, {0.3f, 0.8f, 1, 1}},
        {{-1, 1, 1}, {0, 1}, {0.3f, 1, 0.7f, 1}},
    }};

    const std::array<uint16_t, 36> indices{{0, 2, 1, 0, 3, 2, 4, 5, 6, 4, 6, 7,
                                            8, 9, 10, 8, 10, 11, 12, 15, 14, 12, 14, 13,
                                            16, 17, 18, 16, 18, 19, 20, 23, 22, 20, 22, 21}};
    m_vertices = Rendering::Buffer::Upload(renderer.GetDevice(), std::as_bytes(std::span(vertices)), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    m_indices  = Rendering::Buffer::Upload(renderer.GetDevice(), std::as_bytes(std::span(indices)), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    auto&                  device   = renderer.GetDevice();
    int                    texWidth = 1, texHeight = 1, texChannels = 0;
    stbi_uc*               texPixels = nullptr;
    std::vector<std::byte> texFile;
    if (vfs.Exists("textures/test.png"))
    {
        texFile   = vfs.ReadBytes("textures/test.png");
        texPixels = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(texFile.data()),
                                          static_cast<int>(texFile.size()), &texWidth, &texHeight, &texChannels, 4);
        PICO_ASSERT(texPixels != nullptr, "stb cannot decode textures/test.png: {}", stbi_failure_reason());
    }
    const std::array<std::byte, 4>   white{std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}};
    const std::span<const std::byte> upload = texPixels != nullptr
                                                  ? std::span<const std::byte>(reinterpret_cast<const std::byte*>(texPixels),
                                                                               static_cast<size_t>(texWidth) * static_cast<size_t>(texHeight) * 4)
                                                  : std::span<const std::byte>(white);
    m_texture                               = Rendering::Texture2D::Upload(device, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), upload);
    if (texPixels != nullptr) stbi_image_free(texPixels);

    VkSamplerCreateInfo samplerInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.magFilter     = VK_FILTER_LINEAR;
    samplerInfo.minFilter     = VK_FILTER_LINEAR;
    samplerInfo.addressModeU  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.maxLod        = 0.0f;
    Check(vkCreateSampler(device.Handle(), &samplerInfo, nullptr, &m_sampler), "Create sample sampler");

    VkDescriptorSetLayoutBinding setBinding{};
    setBinding.binding         = 0;
    setBinding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setBinding.descriptorCount = 1;
    setBinding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo setInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    setInfo.bindingCount = 1;
    setInfo.pBindings    = &setBinding;
    Check(vkCreateDescriptorSetLayout(device.Handle(), &setInfo, nullptr, &m_setLayout), "Create sample set layout");

    VkDescriptorPoolSize       poolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};
    VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.maxSets       = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes    = &poolSize;
    Check(vkCreateDescriptorPool(device.Handle(), &poolInfo, nullptr, &m_pool), "Create sample pool");

    VkDescriptorSetAllocateInfo alloc{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    alloc.descriptorPool     = m_pool;
    alloc.descriptorSetCount = 1;
    alloc.pSetLayouts        = &m_setLayout;
    Check(vkAllocateDescriptorSets(device.Handle(), &alloc, &m_set), "Allocate sample set");

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler     = m_sampler;
    imageInfo.imageView   = m_texture->View();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write.dstSet          = m_set;
    write.dstBinding      = 0;
    write.descriptorCount = 1;
    write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo      = &imageInfo;
    vkUpdateDescriptorSets(device.Handle(), 1, &write, 0, nullptr);
    LOG_DEBUG("Sample texture '{}' ({}x{}) bound to set 0", vfs.Exists("textures/test.png") ? "textures/test.png" : "<white fallback>",
              texWidth, texHeight);
}
RotatingCube::~RotatingCube()
{
    vkDeviceWaitIdle(m_renderer.GetDevice().Handle());
    const VkDevice device = m_renderer.GetDevice().Handle();

    m_pipeline.reset();
    m_fragmentShader.reset();
    m_vertexShader.reset();
    m_texture.reset();
    if (m_pool != VK_NULL_HANDLE) vkDestroyDescriptorPool(device, m_pool, nullptr);
    if (m_setLayout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(device, m_setLayout, nullptr);
    if (m_sampler != VK_NULL_HANDLE) vkDestroySampler(device, m_sampler, nullptr);
    m_pool      = VK_NULL_HANDLE;
    m_setLayout = VK_NULL_HANDLE;
    m_sampler   = VK_NULL_HANDLE;
    m_set       = VK_NULL_HANDLE;
}
void RotatingCube::Draw(VkCommandBuffer command, float seconds)
{
    if (m_generation != m_renderer.Generation())
    {
        VkVertexInputBindingDescription                  binding{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
        std::array<VkVertexInputAttributeDescription, 3> attributes{{{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
                                                                     {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)},
                                                                     {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color)}}};

        const VkSpecializationMapEntry  useMapEntry{0, 0, sizeof(int32_t)};
        const int32_t                   useMapValue = 1;
        Rendering::GraphicsPipelineDesc desc{m_renderer.RenderPass(),
                                             *m_vertexShader,
                                             *m_fragmentShader,
                                             std::span(&binding, 1),
                                             attributes,
                                             sizeof(UnlitPush),
                                             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                             {},
                                             {},
                                             std::span(&useMapEntry, 1),
                                             std::as_bytes(std::span(&useMapValue, 1)),
                                             std::span(&m_setLayout, 1)};
        m_pipeline   = std::make_unique<Rendering::GraphicsPipeline>(m_renderer.GetDevice(), desc);
        m_generation = m_renderer.Generation();
    }
    auto extent     = m_renderer.Extent();
    auto projection = glm::perspectiveRH_ZO(glm::radians(50.0f), float(extent.width) / float(extent.height), 0.1f, 100.0f);
    projection[1][1] *= -1.0f;
    auto            view  = glm::lookAtRH(glm::vec3(0, 1.5f, 6), glm::vec3(0), glm::vec3(0, 1, 0));
    auto            model = glm::rotate(glm::mat4(1), seconds * 0.8f, glm::normalize(glm::vec3(0.4f, 1, 0.2f)));
    glm::mat4       mvp   = projection * view * model;
    const UnlitPush push{mvp, glm::vec4(1.0f), glm::vec4(1, 1, 0, 0)};
    vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->Handle());
    vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->Layout(), 0, 1, &m_set, 0, nullptr);
    VkBuffer     vertex = m_vertices->Handle();
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(command, 0, 1, &vertex, &offset);
    vkCmdBindIndexBuffer(command, m_indices->Handle(), 0, VK_INDEX_TYPE_UINT16);
    vkCmdPushConstants(command, m_pipeline->Layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                       sizeof(push), &push);
    vkCmdDrawIndexed(command, 36, 1, 0, 0, 0);
}
} // namespace PicoEngine
