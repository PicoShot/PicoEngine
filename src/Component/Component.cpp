#include "Component.hpp"
#include "GameObject/GameObject.hpp"
#include "Scene/Scene.hpp"
#include "Transform/Transform.hpp"

namespace PicoEngine
{

GameObject Component::GetGameObject()
{
    PICO_ASSERT(m_scene != nullptr, "Component has no owner");
    return GameObject(m_scene, m_entity);
}

GameObject Component::GetGameObject() const
{
    PICO_ASSERT(m_scene != nullptr, "Component has no owner");
    return GameObject(m_scene, m_entity);
}

Scene& Component::GetScene()
{
    PICO_ASSERT(m_scene != nullptr, "Component has no owner");
    return *m_scene;
}

Transform& Component::GetTransform()
{
    return *GetGameObject().GetComponent<Transform>();
}

const Transform& Component::GetTransform() const
{
    return *GetGameObject().GetComponent<Transform>();
}

} // namespace PicoEngine
