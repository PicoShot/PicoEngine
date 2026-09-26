#pragma once

namespace PicoEngine
{

struct NameComponent
{
    std::string name;
};

struct ActiveComponent
{
    bool active = true;
};

struct ParentComponent
{
    entt::entity parent = entt::null;
};

struct ChildrenComponent
{
    std::vector<entt::entity> children;
};

struct DestroyRequest
{
};

} // namespace PicoEngine
