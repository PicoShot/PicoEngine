---@meta
-- GameObject: lightweight handle to a scene entity.

---@class GameObject
GameObject = {}

---@return boolean false once the object is destroyed
function GameObject:IsValid() end

---@return string
function GameObject:GetName() end

---@param name string
function GameObject:SetName(name) end

---@return boolean own active flag, ignoring parents
function GameObject:IsActiveSelf() end

---@param active boolean
function GameObject:SetActive(active) end

---@return boolean false if this object or any ancestor is inactive
function GameObject:IsActiveInHierarchy() end

---@return Transform|nil nil when the handle is invalid
function GameObject:GetTransform() end

---@return GameObject invalid handle when unparented or invalid
function GameObject:GetParent() end

---@return GameObject[] empty when invalid
function GameObject:GetChildren() end

---@return integer 0 when invalid
function GameObject:GetChildCount() end

---@param index integer 0-based
---@return GameObject invalid handle when out of range or invalid
function GameObject:GetChild(index) end

---@return GameObject topmost ancestor (or self), invalid when invalid
function GameObject:GetRoot() end

---@param other GameObject
---@return boolean true when `other` is an ancestor of this object
function GameObject:IsChildOf(other) end

---@param parent GameObject new parent; an invalid handle unparents
---@param keepWorldPosition? boolean default true: preserve world transform
function GameObject:SetParent(parent, keepWorldPosition) end

---Deferred destruction, processed at the end of the frame with the whole subtree.
function GameObject:Destroy() end
