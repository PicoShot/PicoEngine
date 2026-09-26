#include "Camera.hpp"
#include "Transform/Transform.hpp"
#include "Debug/Debug.hpp"

namespace PicoEngine
{

glm::mat4 Camera::GetViewMatrix() const
{
    const glm::mat4& world   = GetTransform().worldMatrix;
    const glm::vec3  eye     = glm::vec3(world[3]);
    const glm::vec3  forward = glm::normalize(glm::vec3(world * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
    const glm::vec3  up      = glm::normalize(glm::vec3(world * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));
    return glm::lookAtRH(eye, eye + forward, up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspect) const
{
    PICO_ASSERT(aspect > 0.0f, "Camera projection needs a positive aspect ratio");
    PICO_ASSERT(nearClipPlane > 0.0f && farClipPlane > nearClipPlane, "Camera has an invalid clip range");
    glm::mat4 projection;
    if (mode == ProjectionMode::Perspective)
    {
        projection =
            glm::perspectiveRH_ZO(glm::radians(fieldOfView), aspect, nearClipPlane, farClipPlane);
    }
    else
    {
        const float halfWidth = orthographicSize * aspect;
        projection            = glm::orthoRH_ZO(-halfWidth, halfWidth, -orthographicSize, orthographicSize,
                                                nearClipPlane, farClipPlane);
    }
    projection[1][1] *= -1.0f; // Vulkan NDC has Y down
    return projection;
}

} // namespace PicoEngine
