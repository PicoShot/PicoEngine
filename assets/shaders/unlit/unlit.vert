#version 450

#include "../commons/common.unlit.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 color;

layout(location = 0) out vec2 vUv;
layout(location = 1) out vec4 vColor;

void main()
{
    gl_Position = pico.mvp * vec4(position, 1.0);
    vUv         = uv;
    vColor      = color;
}
