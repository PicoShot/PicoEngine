#version 450

#include "../commons/common.unlit.glsl"

layout(location = 0) in vec2 vUv;
layout(location = 1) in vec4 vColor;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D albedoMap;

layout(constant_id = 0) const int USE_MAP = 1;

void main()
{
    vec2 uv     = PicoTransformUv(vUv, pico.mapST);
    vec4 albedo = USE_MAP != 0 ? texture(albedoMap, uv) : vec4(1.0);
    outColor    = albedo * pico.tint * vColor;
}
