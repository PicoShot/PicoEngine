---@meta
-- Transform: local-space position/rotation/scale with a cached world matrix.

---@class Transform
Transform = {}

---@return vec3 local position
function Transform:GetPosition() end

---@overload fun(x: number, y: number, z: number)
---@param value vec3 new local position
function Transform:SetPosition(value) end

---@return vec3 local scale
function Transform:GetScale() end

---@overload fun(x: number, y: number, z: number)
---@param value vec3 new local scale
function Transform:SetScale(value) end

---@overload fun(x: number, y: number, z: number)
---@param delta vec3 local-space offset added to the position
function Transform:Translate(delta) end

---@param axis vec3 rotation axis (need not be normalized)
---@param angleRadians number rotation angle in radians
function Transform:Rotate(axis, angleRadians) end

---@return vec3 world-space position, updated every frame by the Scene
function Transform:GetWorldPosition() end

---@return vec3 local +Z axis in world space (Unity convention)
function Transform:GetForward() end

---@return vec3 local +Y axis in world space
function Transform:GetUp() end

---@return vec3 local +X axis in world space
function Transform:GetRight() end
