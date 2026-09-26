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

void Transform::LookAt(const glm::vec3& target)
{
    const glm::vec3 direction = target - position;
    if (glm::dot(direction, direction) < 1e-10f)
        return;

    const glm::vec3 forward = glm::normalize(direction);
    glm::vec3       worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    if (std::fabs(glm::dot(forward, worldUp)) > 0.999f)
        worldUp = glm::vec3(1.0f, 0.0f, 0.0f);

    const glm::vec3 right = glm::normalize(glm::cross(worldUp, forward));
    const glm::vec3 up    = glm::cross(forward, right);
    rotation              = glm::normalize(
        glm::quat_cast(glm::mat4(glm::vec4(right, 0.0f), glm::vec4(up, 0.0f), glm::vec4(forward, 0.0f),
                                 glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))));
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
