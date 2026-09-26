#pragma once

namespace PicoEngine
{

class GameObject;

class Scene
{
  public:
    explicit Scene(std::string name = "Scene");
    ~Scene();

    Scene(const Scene&)            = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&)                 = delete;
    Scene& operator=(Scene&&)      = delete;

    const std::string& GetName() const
    {
        return m_name;
    }

    GameObject CreateGameObject(const std::string& name);
    GameObject CreateGameObject(const std::string& name, GameObject parent);
    void       DestroyGameObject(GameObject object);
    void       ProcessDestroyQueue();
    void       Clear();

    void Update(float deltaTime); // Behaviours, then transforms, then deferred destruction
    void UpdateBehaviours(float deltaTime);
    void UpdateTransforms();

    GameObject              FindByName(const std::string& name) const;
    std::vector<GameObject> GetRootGameObjects() const;
    size_t                  GetGameObjectCount() const;

    bool            IsValid(entt::entity entity) const;
    entt::registry& GetRegistry()
    {
        return m_registry;
    }

  private:
    friend class GameObject;
    static void InvokeOnDestroy(entt::registry& registry, entt::entity entity);
    void      SetParent(entt::entity child, entt::entity parent, bool keepWorldPosition);
    glm::mat4 ComputeWorldMatrix(entt::entity entity) const;
    void      CollectSubtree(entt::entity root, std::vector<entt::entity>& out) const;

    std::string    m_name;
    entt::registry m_registry;
};

} // namespace PicoEngine
