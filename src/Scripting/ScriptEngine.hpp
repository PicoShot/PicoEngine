#pragma once

namespace PicoEngine
{

class GameObject;
class Scene;

namespace IO
{
class Vfs;
}

namespace Rendering
{
class Device;
}

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
    void SetRenderContext(Rendering::Device* device, IO::Vfs* vfs);

    bool ExecuteString(const std::string& source, const std::string& chunkName = "chunk");
    bool ExecuteFile(const std::filesystem::path& path);

    GameObject AddLuaBehaviour(GameObject target, const std::string& source,
                               const std::string& chunkName = "chunk");

    void AttachBehaviour(GameObject target, sol::table behaviour);

  private:
    void RegisterTypes();
    bool ReportResult(const sol::protected_function_result& result, const std::string& what);

    sol::state         m_lua;
    Scene*             m_scene  = nullptr;
    Rendering::Device* m_device = nullptr;
    IO::Vfs*           m_vfs    = nullptr;
};

} // namespace PicoEngine
