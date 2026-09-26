---@meta
-- Scene: owns every GameObject. Scripts reach it through the `scene` global,
-- refreshed by ScriptEngine::SetActiveScene.

---@class Scene
Scene = {}

---@overload fun(name: string): GameObject
---@param name string
---@param parent GameObject
---@return GameObject
function Scene:CreateGameObject(name, parent) end

---@param name string first object with a matching name
---@return GameObject invalid handle when nothing matches
function Scene:FindByName(name) end

---@return integer
function Scene:GetGameObjectCount() end

---@type Scene the active scene
scene = nil
