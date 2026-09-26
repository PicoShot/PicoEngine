#pragma once

#include "Component/Component.hpp"

namespace PicoEngine
{

class Behaviour : public Component
{
  public:
    virtual ~Behaviour() = default;

    virtual void Awake();
    virtual void Start();
    virtual void Update(float deltaTime)
    {
        (void)deltaTime;
    }
    virtual void OnDestroy();
};

struct BehaviourList
{
    struct Entry
    {
        std::shared_ptr<Behaviour> behaviour;
        bool                       started = false;
    };

    std::vector<Entry> entries;
};

} // namespace PicoEngine
