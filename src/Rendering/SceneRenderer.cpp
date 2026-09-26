#include "SceneRenderer.hpp"
#include "GameObject/GameObject.hpp"
#include "Rendering/Camera.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/MeshRenderer.hpp"
#include "Rendering/Renderer.hpp"
#include "Scene/Scene.hpp"
#include "Transform/Transform.hpp"

namespace PicoEngine::Rendering
{

void SceneRenderer::Render(Renderer& renderer, Scene& scene, VkCommandBuffer command)
{
    const GameObject cameraObject = scene.GetMainCamera();
    if (!cameraObject.IsValid())
        return;
    const Camera&    camera = *cameraObject.GetComponent<Camera>();
    const VkExtent2D extent = renderer.Extent();
    if (extent.width == 0 || extent.height == 0)
        return;
    const float     aspect         = static_cast<float>(extent.width) / static_cast<float>(extent.height);
    const glm::mat4 viewProjection = camera.GetProjectionMatrix(aspect) * camera.GetViewMatrix();

    for (auto [entity, meshRenderer, transform] : scene.GetRegistry().view<MeshRenderer, Transform>().each())
    {
        const GameObject object(&scene, entity);
        if (!object.IsActiveInHierarchy())
            continue;
        if (meshRenderer.mesh == nullptr || meshRenderer.material == nullptr)
            continue;
        meshRenderer.material->Draw(command, renderer, *meshRenderer.mesh, viewProjection * transform.worldMatrix);
    }
}

} // namespace PicoEngine::Rendering
