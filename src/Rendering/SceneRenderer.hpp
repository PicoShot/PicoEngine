#pragma once

namespace PicoEngine
{

class Scene;

namespace Rendering
{

class Renderer;

class SceneRenderer
{
  public:
    static void Render(Renderer& renderer, Scene& scene, VkCommandBuffer command);
};

} // namespace Rendering
} // namespace PicoEngine
