#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 color;

layout(location = 0) out vec2 vUv;
layout(location = 1) out vec4 vColor;

layout(push_constant) uniform Push
{
    mat4 mvp;
    vec4 tint;
    vec4 mapST;
} push;

void main()
{
    gl_Position = push.mvp * vec4(position, 1.0);
    vUv         = uv;
    vColor      = color;
}
