---@meta
-- vec3
-- Construct with vec3.new(...) or the call shorthand vec3(...).

---@class vec3
---@field x number
---@field y number
---@field z number
---@operator add(vec3): vec3
---@operator sub(vec3): vec3
---@operator unm: vec3
---@operator mul(number): vec3
---@operator div(number): vec3
---@overload fun(): vec3
---@overload fun(x: number, y: number, z: number): vec3
vec3 = {}

---@param x number
---@param y number
---@param z number
---@return vec3
function vec3.new(x, y, z) end

---@return number euclidean length
function vec3:Length() end

---@return vec3 unit vector in the same direction
function vec3:Normalized() end

---@param other vec3
---@return number
function vec3:Dot(other) end

---@param other vec3
---@return vec3
function vec3:Cross(other) end
