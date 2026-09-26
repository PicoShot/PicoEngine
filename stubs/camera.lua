---@meta
-- Camera: perspective/orthographic view onto the scene

---@class Camera
Camera = {}

---@type boolean preferred by Scene::GetMainCamera
Camera.main = false

---@type number vertical field of view in degrees (perspective only)
Camera.fieldOfView = 50.0

---@type number half-height in world units (orthographic only)
Camera.orthographicSize = 5.0

---@type number near clip distance, must be positive
Camera.nearClipPlane = 0.1

---@type number far clip distance, must exceed the near plane
Camera.farClipPlane = 100.0

---Switch to perspective projection.
function Camera:SetPerspective() end

---Switch to orthographic projection.
function Camera:SetOrthographic() end
