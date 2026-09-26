local camObj = scene:CreateGameObject("MainCamera")
local cam = camObj:AddCamera()
cam.main = true
cam.fieldOfView = 50.0

local camTransform = camObj:GetTransform()
camTransform:SetPosition(0.0, 1.5, 6.0)
camTransform:LookAt(0.0, 0.0, 0.0)

local cube = scene:CreateGameObject("Cube")
local cubeRenderer = cube:AddMeshRenderer()
cubeRenderer:SetMesh(Mesh.Cube())
cubeRenderer:SetMaterial(Material.Unlit("textures/test.png"))

local spinner = {}
spinner.axis = vec3(0.4, 1.0, 0.2):Normalized()
function spinner:Update(dt)
    self.gameObject:GetTransform():Rotate(self.axis, 0.8 * dt)
end
AttachBehaviour(cube, spinner)

LogDebug("sample scene loaded")
