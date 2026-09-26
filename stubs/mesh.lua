---@meta
-- Rendering resources and the MeshRenderer component.

---@class Mesh opaque GPU geometry, no direct API
Mesh = {}

---@return Mesh a 2-unit cube centered on the origin (position/uv/color)
function Mesh.Cube() end

---@class Material unlit material: albedo texture with white fallback + tint
Material = {}

---@overload fun(): Material white, no texture
---@param texturePath string VFS path, e.g. "textures/test.png"
---@return Material
function Material.Unlit(texturePath) end

---@return vec4
function Material:GetTint() end

---@param tint vec4
function Material:SetTint(tint) end

---@class MeshRenderer pairs a Mesh with a Material on a GameObject.
---There is no constructor: use gameObject:AddMeshRenderer().
MeshRenderer = {}

---@param mesh Mesh
function MeshRenderer:SetMesh(mesh) end

---@param material Material
function MeshRenderer:SetMaterial(material) end

---@return boolean
function MeshRenderer:HasMesh() end

---@return boolean
function MeshRenderer:HasMaterial() end

---@param target GameObject
---@param behaviour BehaviourScript table attached with `gameObject` injected
function AttachBehaviour(target, behaviour) end
