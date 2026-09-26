#pragma once

namespace PicoEngine
{

class GameObject;
class Scene;
struct Transform;

class Component
{
  public:
    GameObject GetGameObject();
    Scene&     GetScene();
    Transform& GetTransform();

  protected:
    Component() = default;

    friend class GameObject;
    friend class Scene;

    Scene*       m_scene  = nullptr;
    entt::entity m_entity = entt::null;
};

} // namespace PicoEngine
