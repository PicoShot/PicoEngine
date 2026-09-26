#pragma once

#include "Component/Component.hpp"

namespace PicoEngine
{

enum class ProjectionMode
{
    Perspective,
    Orthographic
};

struct Camera : public Component
{
    ProjectionMode mode             = ProjectionMode::Perspective;
    bool           main             = false;
    float          fieldOfView      = 50.0f;
    float          orthographicSize = 5.0f;
    float          nearClipPlane    = 0.1f;
    float          farClipPlane     = 100.0f;

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspect) const;
};

} // namespace PicoEngine
