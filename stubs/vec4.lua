---@meta
-- vec4
-- Construct with vec4.new(...) or the call shorthand vec4(...).

---@class vec4
---@field x number
---@field y number
---@field z number
---@field w number
---@operator add(vec4): vec4
---@operator sub(vec4): vec4
---@operator unm: vec4
---@operator mul(number): vec4
---@operator div(number): vec4
---@operator eq(vec4): boolean
---@overload fun(): vec4
---@overload fun(x: number, y: number, z: number, w: number): vec4
vec4 = {}

---@param x number
---@param y number
---@param z number
---@param w number
---@return vec4
function vec4.new(x, y, z, w) end

---@return number euclidean length
function vec4:Length() end

---@return vec4 unit vector in the same direction
function vec4:Normalized() end

---@param other vec4
---@return number
function vec4:Dot(other) end
