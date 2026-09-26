---@meta
-- Engine log functions usable from gameplay scripts.

---@param message string
function LogDebug(message) end

---@param message string
function LogWarning(message) end

---@param message string
function LogError(message) end

---@class BehaviourScript
---@field gameObject GameObject owner, injected by the engine before Awake
BehaviourScript = {}

function BehaviourScript:Awake() end
function BehaviourScript:Start() end

---@param dt number seconds since the last frame
function BehaviourScript:Update(dt) end

function BehaviourScript:OnDestroy() end
