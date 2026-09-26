#pragma once

namespace PicoEngine
{

class GameObject;
class Scene;
struct Transform;

class Component
{
  public:
    GameObject       GetGameObject();
    GameObject       GetGameObject() const;
    Scene&           GetScene();
    Transform&       GetTransform();
    const Transform& GetTransform() const;

  protected:
    Component() = default;

    friend class GameObject;
    friend class Scene;

    Scene*       m_scene  = nullptr;
    entt::entity m_entity = entt::null;
};

} // namespace PicoEngine
