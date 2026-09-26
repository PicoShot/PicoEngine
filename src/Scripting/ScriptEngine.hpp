#pragma once

namespace PicoEngine
{

class GameObject;
class Scene;

class ScriptEngine
{
  public:
    ScriptEngine();
    ~ScriptEngine();

    ScriptEngine(const ScriptEngine&)            = delete;
    ScriptEngine& operator=(const ScriptEngine&) = delete;
    ScriptEngine(ScriptEngine&&)                 = delete;
    ScriptEngine& operator=(ScriptEngine&&)      = delete;

    sol::state& GetState()
    {
        return m_lua;
    }

    void SetActiveScene(Scene* scene);

    bool ExecuteString(const std::string& source, const std::string& chunkName = "chunk");
    bool ExecuteFile(const std::filesystem::path& path);

    GameObject AddLuaBehaviour(GameObject target, const std::string& source,
                               const std::string& chunkName = "chunk");

  private:
    void RegisterTypes();
    bool ReportResult(const sol::protected_function_result& result, const std::string& what);

    sol::state m_lua;
    Scene*     m_scene = nullptr;
};

} // namespace PicoEngine
