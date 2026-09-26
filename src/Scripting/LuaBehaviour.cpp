#include "LuaBehaviour.hpp"
#include "GameObject/GameObject.hpp"

namespace PicoEngine
{

LuaBehaviour::LuaBehaviour(sol::table instance) : m_self(std::move(instance))
{
}

void LuaBehaviour::Awake()
{
    CallHook("Awake");
}

void LuaBehaviour::Start()
{
    CallHook("Start");
}

void LuaBehaviour::Update(float deltaTime)
{
    CallHook("Update", deltaTime);
}

void LuaBehaviour::OnDestroy()
{
    CallHook("OnDestroy");
}

void LuaBehaviour::CallHook(const char* hookName)
{
    try
    {
        const sol::object callback = m_self[hookName];
        if (!callback.valid() || callback.get_type() != sol::type::function)
            return;
        const sol::protected_function hook = callback;
        if (const sol::protected_function_result result = hook(m_self); !result.valid())
        {
            const sol::error error = result;
            LOG_ERROR("LuaBehaviour:{} on '{}' failed: {}", hookName, GetGameObject().GetName(), error.what());
        }
    }
    catch (const std::exception& exception)
    {
        LOG_ERROR("LuaBehaviour:{} failed with exception: {}", hookName, exception.what());
    }
    catch (...)
    {
        LOG_ERROR("LuaBehaviour:{} failed with unknown error", hookName);
    }
}

void LuaBehaviour::CallHook(const char* hookName, float deltaTime)
{
    try
    {
        const sol::object callback = m_self[hookName];
        if (!callback.valid() || callback.get_type() != sol::type::function)
            return;
        const sol::protected_function hook = callback;
        if (const sol::protected_function_result result = hook(m_self, deltaTime); !result.valid())
        {
            const sol::error error = result;
            LOG_ERROR("LuaBehaviour:{} on '{}' failed: {}", hookName, GetGameObject().GetName(), error.what());
        }
    }
    catch (const std::exception& exception)
    {
        LOG_ERROR("LuaBehaviour:{} failed with exception: {}", hookName, exception.what());
    }
    catch (...)
    {
        LOG_ERROR("LuaBehaviour:{} failed with unknown error", hookName);
    }
}

} // namespace PicoEngine
