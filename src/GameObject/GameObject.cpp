#include "GameObject.hpp"
#include "Scene/Hierarchy.hpp"
#include "Transform/Transform.hpp"

namespace PicoEngine
{

bool GameObject::IsValid() const
{
    return m_scene != nullptr && m_scene->IsValid(m_entity);
}

std::string GameObject::GetName() const
{
    PICO_ASSERT(IsValid(), "GetName on invalid GameObject");
    return m_scene->GetRegistry().get<NameComponent>(m_entity).name;
}

void GameObject::SetName(const std::string& name)
{
    PICO_ASSERT(IsValid(), "SetName on invalid GameObject");
    m_scene->GetRegistry().get<NameComponent>(m_entity).name = name;
}

bool GameObject::IsActiveSelf() const
{
    PICO_ASSERT(IsValid(), "IsActiveSelf on invalid GameObject");
    return m_scene->GetRegistry().get<ActiveComponent>(m_entity).active;
}

void GameObject::SetActive(bool active)
{
    PICO_ASSERT(IsValid(), "SetActive on invalid GameObject");
    m_scene->GetRegistry().get<ActiveComponent>(m_entity).active = active;
}

bool GameObject::IsActiveInHierarchy() const
{
    PICO_ASSERT(IsValid(), "IsActiveInHierarchy on invalid GameObject");
    entt::registry& registry = m_scene->GetRegistry();
    entt::entity    current  = m_entity;
    while (current != entt::null)
    {
        if (!registry.get<ActiveComponent>(current).active)
            return false;
        const ParentComponent* parent = registry.try_get<ParentComponent>(current);
        current                       = (parent != nullptr) ? parent->parent : entt::null;
    }
    return true;
}

Transform& GameObject::GetTransform()
{
    return *GetComponent<Transform>();
}

const Transform& GameObject::GetTransform() const
{
    return *GetComponent<Transform>();
}

GameObject GameObject::GetParent() const
{
    PICO_ASSERT(IsValid(), "GetParent on invalid GameObject");
    const ParentComponent* parent = m_scene->GetRegistry().try_get<ParentComponent>(m_entity);
    if (parent == nullptr || !m_scene->IsValid(parent->parent))
        return GameObject();
    return GameObject(m_scene, parent->parent);
}

std::vector<GameObject> GameObject::GetChildren() const
{
    PICO_ASSERT(IsValid(), "GetChildren on invalid GameObject");
    std::vector<GameObject>  result;
    const ChildrenComponent& children = m_scene->GetRegistry().get<ChildrenComponent>(m_entity);
    for (entt::entity child : children.children)
        if (m_scene->IsValid(child))
            result.emplace_back(m_scene, child);
    return result;
}

int GameObject::GetChildCount() const
{
    PICO_ASSERT(IsValid(), "GetChildCount on invalid GameObject");
    return static_cast<int>(m_scene->GetRegistry().get<ChildrenComponent>(m_entity).children.size());
}

GameObject GameObject::GetChild(int index) const
{
    PICO_ASSERT(IsValid(), "GetChild on invalid GameObject");
    const std::vector<entt::entity>& children =
        m_scene->GetRegistry().get<ChildrenComponent>(m_entity).children;
    PICO_ASSERT(index >= 0 && index < static_cast<int>(children.size()), "Child index out of range");
    PICO_ASSERT(m_scene->IsValid(children[index]), "Child no longer exists");
    return GameObject(m_scene, children[index]);
}

GameObject GameObject::GetRoot() const
{
    PICO_ASSERT(IsValid(), "GetRoot on invalid GameObject");
    GameObject root = *this;
    while (true)
    {
        GameObject parent = root.GetParent();
        if (!parent.IsValid())
            return root;
        root = parent;
    }
}

bool GameObject::IsChildOf(GameObject other) const
{
    PICO_ASSERT(IsValid(), "IsChildOf on invalid GameObject");
    GameObject current = GetParent();
    while (current.IsValid())
    {
        if (current == other)
            return true;
        current = current.GetParent();
    }
    return false;
}

void GameObject::SetParent(GameObject parent, bool keepWorldPosition)
{
    PICO_ASSERT(IsValid(), "SetParent on invalid GameObject");
    PICO_ASSERT(!parent.IsValid() || parent.GetScene() == m_scene, "Parent belongs to a different Scene");
    m_scene->SetParent(m_entity, parent.IsValid() ? parent.GetEntity() : entt::null, keepWorldPosition);
}

void GameObject::Destroy()
{
    PICO_ASSERT(IsValid(), "Destroy on invalid GameObject");
    m_scene->DestroyGameObject(*this);
}

} // namespace PicoEngine
