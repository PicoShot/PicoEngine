#pragma once

#include "Component/Component.hpp"
#include "Debug/Debug.hpp"
#include "Scene/Scene.hpp"
#include "Transform/Transform.hpp"

namespace PicoEngine
{

class GameObject
{
  public:
    GameObject() = default;

    bool     IsValid() const;
    explicit operator bool() const
    {
        return IsValid();
    }

    bool operator==(const GameObject& other) const
    {
        return m_scene == other.m_scene && m_entity == other.m_entity;
    }

    bool operator!=(const GameObject& other) const
    {
        return !(*this == other);
    }

    Scene* GetScene() const
    {
        return m_scene;
    }
    entt::entity GetEntity() const
    {
        return m_entity;
    }

    std::string GetName() const;
    void        SetName(const std::string& name);

    bool IsActiveSelf() const;
    void SetActive(bool active);
    bool IsActiveInHierarchy() const;

    Transform&       GetTransform();
    const Transform& GetTransform() const;

    template <typename T, typename... Args>
        requires std::derived_from<T, Component>
    T* AddComponent(Args&&... args)
    {
        PICO_ASSERT(IsValid(), "AddComponent on invalid GameObject");
        PICO_ASSERT(!HasComponent<T>(), "GameObject already has this component");
        T& component       = m_scene->GetRegistry().emplace<T>(m_entity, std::forward<Args>(args)...);
        component.m_scene  = m_scene;
        component.m_entity = m_entity;
        return &component;
    }

    template <typename T>
        requires std::derived_from<T, Component>
    T* GetComponent()
    {
        PICO_ASSERT(HasComponent<T>(), "GameObject does not have this component");
        return &m_scene->GetRegistry().get<T>(m_entity);
    }

    template <typename T>
        requires std::derived_from<T, Component>
    const T* GetComponent() const
    {
        PICO_ASSERT(HasComponent<T>(), "GameObject does not have this component");
        return &m_scene->GetRegistry().get<T>(m_entity);
    }

    template <typename T>
        requires std::derived_from<T, Component>
    T* TryGetComponent()
    {
        if (!IsValid())
            return nullptr;
        return m_scene->GetRegistry().try_get<T>(m_entity);
    }

    template <typename T>
        requires std::derived_from<T, Component>
    bool HasComponent() const
    {
        return IsValid() && m_scene->GetRegistry().all_of<T>(m_entity);
    }

    template <typename T>
        requires std::derived_from<T, Component>
    void RemoveComponent()
    {
        static_assert(!std::same_as<T, Transform>, "Transform cannot be removed from a GameObject");
        PICO_ASSERT(HasComponent<T>(), "GameObject does not have this component");
        m_scene->GetRegistry().remove<T>(m_entity);
    }

    GameObject              GetParent() const;
    std::vector<GameObject> GetChildren() const;
    int                     GetChildCount() const;
    GameObject              GetChild(int index) const;
    GameObject              GetRoot() const;
    bool                    IsChildOf(GameObject other) const;
    void                    SetParent(GameObject parent, bool keepWorldPosition = true);

    void Destroy();

    GameObject(Scene* scene, entt::entity entity) : m_scene(scene), m_entity(entity)
    {
    }

  private:
    Scene*       m_scene  = nullptr;
    entt::entity m_entity = entt::null;
};

} // namespace PicoEngine
