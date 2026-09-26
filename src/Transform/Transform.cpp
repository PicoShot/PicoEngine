#include "Transform.hpp"

namespace PicoEngine
{

glm::mat4 Transform::LocalMatrix() const
{
    return glm::translate(glm::identity<glm::mat4>(), position) * glm::mat4_cast(rotation) *
           glm::scale(glm::identity<glm::mat4>(), scale);
}

void Transform::Translate(const glm::vec3& delta)
{
    position += delta;
}

void Transform::Rotate(const glm::vec3& axis, float angleRadians)
{
    rotation = glm::normalize(glm::angleAxis(angleRadians, glm::normalize(axis)) * rotation);
}

glm::vec3 Transform::GetForward() const
{
    return rotation * glm::vec3(0.0f, 0.0f, 1.0f);
}

glm::vec3 Transform::GetUp() const
{
    return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 Transform::GetRight() const
{
    return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 Transform::GetWorldPosition() const
{
    return glm::vec3(worldMatrix[3]);
}

} // namespace PicoEngine
