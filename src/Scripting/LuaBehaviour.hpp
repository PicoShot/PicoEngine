#pragma once

#include "Component/Behaviour.hpp"

namespace PicoEngine
{

class LuaBehaviour : public Behaviour
{
  public:
    explicit LuaBehaviour(sol::table instance);

    void Awake() override;
    void Start() override;
    void Update(float deltaTime) override;
    void OnDestroy() override;

  private:
    void CallHook(const char* hookName);
    void CallHook(const char* hookName, float deltaTime);

    sol::table m_self;
};

} // namespace PicoEngine
