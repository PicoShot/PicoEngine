#pragma once

#include "Component/Component.hpp"

namespace PicoEngine
{

struct Transform : public Component
{
    glm::vec3 position    = glm::vec3(0.0f);
    glm::quat rotation    = glm::identity<glm::quat>();
    glm::vec3 scale       = glm::vec3(1.0f);
    glm::mat4 worldMatrix = glm::identity<glm::mat4>();

    glm::mat4 LocalMatrix() const;
    void      Translate(const glm::vec3& delta);
    void      Rotate(const glm::vec3& axis, float angleRadians);

    glm::vec3 GetForward() const; // +Z rotated
    glm::vec3 GetUp() const;
    glm::vec3 GetRight() const;
    glm::vec3 GetWorldPosition() const;
};

} // namespace PicoEngine
