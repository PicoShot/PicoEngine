#include "Material.hpp"
#include "IO/Vfs.hpp"
#include "Rendering/Mesh.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/ShaderCompiler.hpp"

namespace PicoEngine::Rendering
{

namespace
{

struct UnlitPush
{
    glm::mat4 mvp;
    glm::vec4 tint;
    glm::vec4 mapST;
};

} // namespace

std::shared_ptr<Material> Material::CreateUnlit(Device& device, IO::Vfs& vfs, const std::string& texturePath)
{
    ShaderCompiler compiler(vfs);
    compiler.AddIncludePrefix("shaders/commons");
    auto vertexSpv   = compiler.CompileFile("shaders/unlit/unlit.vert");
    auto fragmentSpv = compiler.CompileFile("shaders/unlit/unlit.frag");
    auto vertexShader =
        std::make_unique<Shader>(device, vertexSpv.spirv, "shaders/unlit/unlit.vert");
    auto fragmentShader =
        std::make_unique<Shader>(device, fragmentSpv.spirv, "shaders/unlit/unlit.frag");

    int                    texWidth = 1, texHeight = 1, texChannels = 0;
    stbi_uc*               texPixels = nullptr;
    std::vector<std::byte> texFile;
    if (!texturePath.empty() && vfs.Exists(texturePath))
    {
        texFile   = vfs.ReadBytes(texturePath);
        texPixels = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(texFile.data()),
                                          static_cast<int>(texFile.size()), &texWidth, &texHeight, &texChannels, 4);
        PICO_ASSERT(texPixels != nullptr, "stb cannot decode {}: {}", texturePath, stbi_failure_reason());
    }
    const std::array<std::byte, 4>   white{std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}};
    const std::span<const std::byte> upload =
        texPixels != nullptr
            ? std::span<const std::byte>(reinterpret_cast<const std::byte*>(texPixels),
                                         static_cast<size_t>(texWidth) * static_cast<size_t>(texHeight) * 4)
            : std::span<const std::byte>(white);
    auto texture = Texture2D::Upload(device, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), upload);
    if (texPixels != nullptr) stbi_image_free(texPixels);

    auto material = std::shared_ptr<Material>(
        new Material(device, std::move(vertexShader), std::move(fragmentShader), std::move(texture)));
    LOG_DEBUG("Unlit material created (texture '{}')", texturePath.empty() ? "<white fallback>" : texturePath);
    return material;
}

Material::Material(Device& device, std::unique_ptr<Shader> vertexShader, std::unique_ptr<Shader> fragmentShader,
                   std::unique_ptr<Texture2D> texture)
    : m_device(device), m_vertexShader(std::move(vertexShader)),
      m_fragmentShader(std::move(fragmentShader)), m_texture(std::move(texture))
{
    VkSamplerCreateInfo samplerInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.magFilter     = VK_FILTER_LINEAR;
    samplerInfo.minFilter     = VK_FILTER_LINEAR;
    samplerInfo.addressModeU  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.maxLod        = 0.0f;
    Check(vkCreateSampler(device.Handle(), &samplerInfo, nullptr, &m_sampler), "Create material sampler");

    VkDescriptorSetLayoutBinding setBinding{};
    setBinding.binding         = 0;
    setBinding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setBinding.descriptorCount = 1;
    setBinding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo setInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    setInfo.bindingCount = 1;
    setInfo.pBindings    = &setBinding;
    Check(vkCreateDescriptorSetLayout(device.Handle(), &setInfo, nullptr, &m_setLayout), "Create material set layout");

    VkDescriptorPoolSize       poolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};
    VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.maxSets       = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes    = &poolSize;
    Check(vkCreateDescriptorPool(device.Handle(), &poolInfo, nullptr, &m_pool), "Create material pool");

    VkDescriptorSetAllocateInfo alloc{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    alloc.descriptorPool     = m_pool;
    alloc.descriptorSetCount = 1;
    alloc.pSetLayouts        = &m_setLayout;
    Check(vkAllocateDescriptorSets(device.Handle(), &alloc, &m_set), "Allocate material set");

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
}

Material::~Material()
{
    vkDeviceWaitIdle(m_device.Handle());
    const VkDevice device = m_device.Handle();
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

void Material::Draw(VkCommandBuffer command, Renderer& renderer, const Mesh& mesh, const glm::mat4& mvp)
{
    EnsurePipeline(renderer);
    vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->Handle());
    vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->Layout(), 0, 1, &m_set, 0, nullptr);
    VkBuffer     vertex = mesh.VertexBuffer();
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(command, 0, 1, &vertex, &offset);
    vkCmdBindIndexBuffer(command, mesh.IndexBuffer(), 0, mesh.IndexType());
    const UnlitPush push{mvp, m_tint, m_mapST};
    vkCmdPushConstants(command, m_pipeline->Layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                       sizeof(push), &push);
    vkCmdDrawIndexed(command, mesh.IndexCount(), 1, 0, 0, 0);
}

void Material::EnsurePipeline(Renderer& renderer)
{
    if (m_generation == renderer.Generation())
        return;
    VkVertexInputBindingDescription                  binding    = Mesh::BindingDescription();
    std::array<VkVertexInputAttributeDescription, 3> attributes = Mesh::AttributeDescriptions();
    const VkSpecializationMapEntry                   useMapEntry{0, 0, sizeof(int32_t)};
    const int32_t                                    useMapValue = 1;
    const GraphicsPipelineDesc                       desc{renderer.RenderPass(), *m_vertexShader, *m_fragmentShader, std::span(&binding, 1), attributes, sizeof(UnlitPush), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, {}, {}, std::span(&useMapEntry, 1), std::as_bytes(std::span(&useMapValue, 1)), std::span(&m_setLayout, 1)};
    m_pipeline   = std::make_unique<GraphicsPipeline>(m_device, desc);
    m_generation = renderer.Generation();
}

} // namespace PicoEngine::Rendering
