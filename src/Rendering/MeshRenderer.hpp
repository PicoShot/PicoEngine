#pragma once

#include "Component/Component.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/Mesh.hpp"

namespace PicoEngine
{

struct MeshRenderer : public Component
{
    std::shared_ptr<Rendering::Mesh>     mesh;
    std::shared_ptr<Rendering::Material> material;
};

} // namespace PicoEngine
