#include "ScriptEngine.hpp"
#include "GameObject/GameObject.hpp"
#include "Scene/Scene.hpp"
#include "Scripting/LuaBehaviour.hpp"
#include "Transform/Transform.hpp"

namespace PicoEngine
{

ScriptEngine::ScriptEngine()
{
    m_lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::math, sol::lib::string, sol::lib::table);
    RegisterTypes();
    LOG_DEBUG("ScriptEngine initialized");
}

ScriptEngine::~ScriptEngine()
{
    LOG_DEBUG("ScriptEngine shut down");
}

void ScriptEngine::SetActiveScene(Scene* scene)
{
    m_scene = scene;
    if (m_scene != nullptr)
        m_lua["scene"] = m_scene;
    else
        m_lua["scene"] = sol::lua_nil;
}

bool ScriptEngine::ExecuteString(const std::string& source, const std::string& chunkName)
{
    try
    {
        return ReportResult(m_lua.safe_script(source, sol::script_pass_on_error), chunkName);
    }
    catch (const std::exception& exception)
    {
        LOG_ERROR("Lua {} failed with exception: {}", chunkName, exception.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR("Lua {} failed with unknown error", chunkName);
        return false;
    }
}

bool ScriptEngine::ExecuteFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        LOG_ERROR("Lua script not found: {}", path.string());
        return false;
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return ExecuteString(contents.str(), path.string());
}

GameObject ScriptEngine::AddLuaBehaviour(GameObject target, const std::string& source, const std::string& chunkName)
{
    PICO_ASSERT(target.IsValid(), "AddLuaBehaviour on invalid GameObject");
    sol::protected_function_result result;
    try
    {
        result = m_lua.safe_script(source, sol::script_pass_on_error);
    }
    catch (const std::exception& exception)
    {
        LOG_ERROR("Lua {} failed with exception: {}", chunkName, exception.what());
        return GameObject();
    }
    if (!result.valid())
    {
        ReportResult(result, chunkName);
        return GameObject();
    }
    const sol::object returned = result.get<sol::object>(0);
    if (!returned.valid() || returned.get_type() != sol::type::table)
    {
        LOG_ERROR("Lua {} must return a behaviour table, got {}", chunkName,
                  returned.valid() ? "a non-table value" : "no value");
        return GameObject();
    }

    sol::table instance = m_lua.create_table();
    instance[sol::metatable_key] =
        m_lua.create_table_with(sol::meta_function::index, returned.as<sol::table>());
    instance["gameObject"] = target;
    target.AddBehaviour<LuaBehaviour>(instance);
    return target;
}

bool ScriptEngine::ReportResult(const sol::protected_function_result& result, const std::string& what)
{
    if (result.valid())
        return true;
    const sol::error error = result;
    LOG_ERROR("Lua {} failed: {}", what, error.what());
    return false;
}

void ScriptEngine::RegisterTypes()
{
    constexpr const char* kCallableBootstrap = R"(
        local function makeCallable(classTable)
            local construct = classTable.new
            return setmetatable({}, {
                __index = classTable,
                __call = function(_, ...) return construct(...) end,
            })
        end
        vec3 = makeCallable(vec3)
        vec4 = makeCallable(vec4)
    )";

    m_lua["LogDebug"]   = [](const std::string& message) { Debug::LogDebug("{}", message); };
    m_lua["LogWarning"] = [](const std::string& message) { Debug::LogWarning("{}", message); };
    m_lua["LogError"]   = [](const std::string& message) { Debug::LogError("{}", message); };

    m_lua.new_usertype<glm::vec3>(
        "vec3", sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(), "x", &glm::vec3::x,
        "y", &glm::vec3::y, "z", &glm::vec3::z, sol::meta_function::addition,
        [](const glm::vec3& a, const glm::vec3& b) { return a + b; }, sol::meta_function::subtraction,
        [](const glm::vec3& a, const glm::vec3& b) { return a - b; }, sol::meta_function::unary_minus,
        [](const glm::vec3& v) { return -v; }, sol::meta_function::multiplication,
        sol::overload([](const glm::vec3& v, float s) { return v * s; },
                      [](float s, const glm::vec3& v) { return v * s; }),
        sol::meta_function::division, [](const glm::vec3& v, float s) { return v / s; },
        sol::meta_function::equal_to, [](const glm::vec3& a, const glm::vec3& b) { return a == b; },
        "Length", [](const glm::vec3& v) { return glm::length(v); }, "Normalized",
        [](const glm::vec3& v) { return glm::normalize(v); }, "Dot",
        [](const glm::vec3& a, const glm::vec3& b) { return glm::dot(a, b); }, "Cross",
        [](const glm::vec3& a, const glm::vec3& b) { return glm::cross(a, b); });

    m_lua.new_usertype<glm::vec4>(
        "vec4", sol::constructors<glm::vec4(), glm::vec4(float, float, float, float)>(), "x", &glm::vec4::x,
        "y", &glm::vec4::y, "z", &glm::vec4::z, "w", &glm::vec4::w, sol::meta_function::addition,
        [](const glm::vec4& a, const glm::vec4& b) { return a + b; }, sol::meta_function::subtraction,
        [](const glm::vec4& a, const glm::vec4& b) { return a - b; }, sol::meta_function::unary_minus,
        [](const glm::vec4& v) { return -v; }, sol::meta_function::multiplication,
        sol::overload([](const glm::vec4& v, float s) { return v * s; },
                      [](float s, const glm::vec4& v) { return v * s; }),
        sol::meta_function::division, [](const glm::vec4& v, float s) { return v / s; },
        sol::meta_function::equal_to, [](const glm::vec4& a, const glm::vec4& b) { return a == b; },
        "Length", [](const glm::vec4& v) { return glm::length(v); }, "Normalized",
        [](const glm::vec4& v) { return glm::normalize(v); }, "Dot",
        [](const glm::vec4& a, const glm::vec4& b) { return glm::dot(a, b); });

    m_lua.new_usertype<Transform>(
        "Transform", "GetPosition", [](const Transform& transform) { return transform.position; },
        "SetPosition",
        sol::overload([](Transform& transform, const glm::vec3& value) { transform.position = value; },
                      [](Transform& transform, float x, float y, float z) { transform.position = glm::vec3(x, y, z); }),
        "GetScale", [](const Transform& transform) { return transform.scale; }, "SetScale",
        sol::overload([](Transform& transform, const glm::vec3& value) { transform.scale = value; },
                      [](Transform& transform, float x, float y, float z) { transform.scale = glm::vec3(x, y, z); }),
        "Translate",
        sol::overload([](Transform& transform, const glm::vec3& delta) { transform.Translate(delta); },
                      [](Transform& transform, float x, float y, float z) { transform.Translate(glm::vec3(x, y, z)); }),
        "Rotate",
        [](Transform& transform, const glm::vec3& axis, float angleRadians) { transform.Rotate(axis, angleRadians); },
        "GetWorldPosition", &Transform::GetWorldPosition, "GetForward", &Transform::GetForward,
        "GetUp", &Transform::GetUp, "GetRight", &Transform::GetRight);

    m_lua.new_usertype<GameObject>(
        "GameObject", "IsValid", &GameObject::IsValid, "GetName", &GameObject::GetName, "SetName",
        &GameObject::SetName, "IsActiveSelf", &GameObject::IsActiveSelf, "SetActive",
        &GameObject::SetActive, "IsActiveInHierarchy", &GameObject::IsActiveInHierarchy, "GetTransform",
        [](GameObject& object) -> Transform* { return object.IsValid() ? &object.GetTransform() : nullptr; },
        "GetParent", [](GameObject& object) { return object.IsValid() ? object.GetParent() : GameObject(); },
        "GetChildren",
        [](GameObject& object) {
            if (!object.IsValid())
                return std::vector<GameObject>();
            return object.GetChildren();
        },
        "GetChildCount", [](GameObject& object) { return object.IsValid() ? object.GetChildCount() : 0; },
        "GetChild",
        [](GameObject& object, int index) {
            if (!object.IsValid() || index < 0 || index >= object.GetChildCount())
                return GameObject();
            return object.GetChild(index);
        },
        "GetRoot", [](GameObject& object) { return object.IsValid() ? object.GetRoot() : GameObject(); },
        "IsChildOf",
        [](GameObject& object, const GameObject& other) { return object.IsValid() && object.IsChildOf(other); },
        "SetParent",
        sol::overload([](GameObject& object, GameObject parent) { if (object.IsValid()) object.SetParent(parent); },
                      [](GameObject& object, GameObject parent, bool keepWorldPosition) { if (object.IsValid()) object.SetParent(parent, keepWorldPosition); }),
        "Destroy", [](GameObject& object) { if (object.IsValid()) object.Destroy(); }, sol::meta_function::equal_to,
        [](const GameObject& a, const GameObject& b) { return a == b; });

    m_lua.new_usertype<Scene>(
        "Scene", "CreateGameObject",
        sol::overload([](Scene& scene, const std::string& name) { return scene.CreateGameObject(name); },
                      [](Scene& scene, const std::string& name, GameObject parent) { return scene.CreateGameObject(name, parent); }),
        "FindByName", &Scene::FindByName, "GetGameObjectCount", &Scene::GetGameObjectCount);

    if (!ReportResult(m_lua.safe_script(kCallableBootstrap, sol::script_pass_on_error), "vector bootstrap"))
        PICO_ASSERT_FAIL("Built-in vector bootstrap script failed");
}

} // namespace PicoEngine
