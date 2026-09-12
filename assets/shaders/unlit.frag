#version 450

layout(location = 0) in vec2 vUv;
layout(location = 1) in vec4 vColor;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D albedoMap;

layout(push_constant) uniform Push
{
    mat4 mvp;
    vec4 tint;
    vec4 mapST
} push;

layout(constant_id = 0) const int USE_MAP = 1;

void main()
{
    vec2 uv     = vUv * push.mapST.xy + push.mapST.zw;
    vec4 albedo = USE_MAP != 0 ? texture(albedoMap, uv) : vec4(1.0);
    outColor    = albedo * push.tint * vColor;
}
