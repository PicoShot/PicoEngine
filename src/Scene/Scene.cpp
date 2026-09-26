#include "Scene.hpp"
#include "GameObject/GameObject.hpp"
#include "Scene/Hierarchy.hpp"
#include "Transform/Transform.hpp"

namespace PicoEngine
{

namespace
{

void DecomposeTRS(const glm::mat4& matrix, glm::vec3& outPosition, glm::quat& outRotation, glm::vec3& outScale)
{
    outPosition = glm::vec3(matrix[3]);

    glm::vec3 columns[3] = {glm::vec3(matrix[0]), glm::vec3(matrix[1]), glm::vec3(matrix[2])};
    outScale             = glm::vec3(glm::length(columns[0]), glm::length(columns[1]), glm::length(columns[2]));
    for (int i = 0; i < 3; ++i)
        if (outScale[i] > 0.0f)
            columns[i] /= outScale[i];
    if (glm::dot(glm::cross(columns[0], columns[1]), columns[2]) < 0.0f)
    {
        outScale.x = -outScale.x;
        columns[0] = -columns[0];
    }
    outRotation = glm::normalize(glm::quat_cast(glm::mat4(glm::vec4(columns[0], 0.0f), glm::vec4(columns[1], 0.0f),
                                                          glm::vec4(columns[2], 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))));
}

} // namespace

Scene::Scene(std::string name) : m_name(std::move(name))
{
    LOG_DEBUG("Scene '{}' created", m_name);
}

Scene::~Scene()
{
    m_registry.clear();
    LOG_DEBUG("Scene '{}' destroyed", m_name);
}

GameObject Scene::CreateGameObject(const std::string& name)
{
    return CreateGameObject(name, GameObject());
}

GameObject Scene::CreateGameObject(const std::string& name, GameObject parent)
{
    PICO_ASSERT(!parent.IsValid() || parent.GetScene() == this, "Parent belongs to a different Scene");
    const entt::entity entity = m_registry.create();
    m_registry.emplace<NameComponent>(entity, name);
    m_registry.emplace<ActiveComponent>(entity, true);
    m_registry.emplace<ChildrenComponent>(entity);
    GameObject object(this, entity);
    object.AddComponent<Transform>();
    if (parent.IsValid())
        SetParent(entity, parent.GetEntity(), false);
    UpdateTransforms();
    LOG_DEBUG("Created GameObject '{}' in scene '{}'", name, m_name);
    return object;
}

void Scene::DestroyGameObject(GameObject object)
{
    PICO_ASSERT(object.GetScene() == this, "GameObject belongs to a different Scene");
    if (!IsValid(object.GetEntity()))
        return;
    if (!m_registry.all_of<DestroyRequest>(object.GetEntity()))
        m_registry.emplace<DestroyRequest>(object.GetEntity());
}

void Scene::ProcessDestroyQueue()
{
    std::vector<entt::entity> roots;
    for (entt::entity entity : m_registry.view<DestroyRequest>())
        roots.push_back(entity);

    std::vector<entt::entity> doomed;
    for (entt::entity root : roots)
        if (IsValid(root))
            CollectSubtree(root, doomed);

    for (entt::entity entity : doomed)
    {
        if (!IsValid(entity))
            continue;
        const std::string name = m_registry.get<NameComponent>(entity).name;
        if (const ParentComponent* parent = m_registry.try_get<ParentComponent>(entity);
            parent != nullptr && IsValid(parent->parent))
        {
            std::vector<entt::entity>& siblings = m_registry.get<ChildrenComponent>(parent->parent).children;
            std::erase(siblings, entity);
        }
        m_registry.destroy(entity);
        LOG_DEBUG("Destroyed GameObject '{}' in scene '{}'", name, m_name);
    }
}

void Scene::Clear()
{
    const size_t count = GetGameObjectCount();
    m_registry.clear();
    LOG_DEBUG("Cleared scene '{}' ({} objects)", m_name, count);
}

void Scene::Update()
{
    UpdateTransforms();
    ProcessDestroyQueue();
}

void Scene::UpdateTransforms()
{
    for (auto [entity, transform] : m_registry.view<Transform>().each())
    {
        if (m_registry.all_of<ParentComponent>(entity))
            continue; // Reached through its parent below
        transform.worldMatrix = transform.LocalMatrix();
        std::vector<entt::entity> stack(m_registry.get<ChildrenComponent>(entity).children);
        while (!stack.empty())
        {
            const entt::entity child = stack.back();
            stack.pop_back();
            if (!IsValid(child))
                continue;
            Transform&      childTransform                 = m_registry.get<Transform>(child);
            const glm::mat4 parentWorld                    = m_registry.get<Transform>(m_registry.get<ParentComponent>(child).parent).worldMatrix;
            childTransform.worldMatrix                     = parentWorld * childTransform.LocalMatrix();
            const std::vector<entt::entity>& grandchildren = m_registry.get<ChildrenComponent>(child).children;
            stack.insert(stack.end(), grandchildren.begin(), grandchildren.end());
        }
    }
}

GameObject Scene::FindByName(const std::string& name) const
{
    for (auto [entity, nameComponent] : m_registry.view<NameComponent>().each())
        if (nameComponent.name == name)
            return GameObject(const_cast<Scene*>(this), entity);
    return GameObject();
}

std::vector<GameObject> Scene::GetRootGameObjects() const
{
    std::vector<GameObject> roots;
    for (auto [entity, name] : m_registry.view<NameComponent>(entt::exclude<ParentComponent>).each())
        roots.emplace_back(const_cast<Scene*>(this), entity);
    return roots;
}

size_t Scene::GetGameObjectCount() const
{
    return m_registry.view<NameComponent>().size();
}

bool Scene::IsValid(entt::entity entity) const
{
    return entity != entt::null && m_registry.valid(entity);
}

void Scene::SetParent(entt::entity child, entt::entity parent, bool keepWorldPosition)
{
    PICO_ASSERT(IsValid(child), "SetParent with invalid child");
    PICO_ASSERT(parent == entt::null || IsValid(parent), "SetParent with invalid parent");
    PICO_ASSERT(child != parent, "GameObject cannot parent to itself");

    for (entt::entity ancestor = parent; ancestor != entt::null;)
    {
        if (ancestor == child)
        {
            LOG_ERROR("SetParent rejected: parenting '{}' under its own descendant would create a cycle",
                      m_registry.get<NameComponent>(child).name);
            return;
        }
        const ParentComponent* ancestorParent = m_registry.try_get<ParentComponent>(ancestor);
        ancestor                              = (ancestorParent != nullptr) ? ancestorParent->parent : entt::null;
    }

    const glm::mat4 worldBefore = keepWorldPosition ? ComputeWorldMatrix(child) : glm::identity<glm::mat4>();

    if (const ParentComponent* oldParent = m_registry.try_get<ParentComponent>(child); oldParent != nullptr)
    {
        if (IsValid(oldParent->parent))
            std::erase(m_registry.get<ChildrenComponent>(oldParent->parent).children, child);
        m_registry.remove<ParentComponent>(child);
    }

    if (parent != entt::null)
    {
        m_registry.emplace<ParentComponent>(child, parent);
        m_registry.get_or_emplace<ChildrenComponent>(parent).children.push_back(child);
    }

    if (keepWorldPosition)
    {
        const glm::mat4 parentWorld =
            parent != entt::null ? ComputeWorldMatrix(parent) : glm::identity<glm::mat4>();
        const glm::mat4 local     = glm::inverse(parentWorld) * worldBefore;
        Transform&      transform = m_registry.get<Transform>(child);
        DecomposeTRS(local, transform.position, transform.rotation, transform.scale);
        transform.worldMatrix = worldBefore;
    }

    UpdateTransforms();
}

glm::mat4 Scene::ComputeWorldMatrix(entt::entity entity) const
{
    glm::mat4 world = glm::identity<glm::mat4>();
    for (entt::entity current = entity; current != entt::null;)
    {
        const Transform* transform = m_registry.try_get<Transform>(current);
        if (transform == nullptr)
        {
            LOG_WARNING("GameObject '{}' has no Transform; treating as identity",
                        m_registry.get<NameComponent>(current).name);
            break;
        }
        world                         = transform->LocalMatrix() * world;
        const ParentComponent* parent = m_registry.try_get<ParentComponent>(current);
        current                       = (parent != nullptr && IsValid(parent->parent)) ? parent->parent : entt::null;
    }
    return world;
}

void Scene::CollectSubtree(entt::entity root, std::vector<entt::entity>& out) const
{
    std::vector<entt::entity> stack{root};
    while (!stack.empty())
    {
        const entt::entity entity = stack.back();
        stack.pop_back();
        if (!IsValid(entity))
            continue;
        out.push_back(entity);
        const std::vector<entt::entity>& children = m_registry.get<ChildrenComponent>(entity).children;
        stack.insert(stack.end(), children.begin(), children.end());
    }
}

} // namespace PicoEngine
