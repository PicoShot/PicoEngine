#ifndef PICO_COMMON_GLSL
#define PICO_COMMON_GLSL

layout(push_constant) uniform PicoPush
{
    mat4 mvp;   // model-view-projection
    vec4 tint;  // xyz = color multiplier, w = alpha multiplier
    vec4 mapST; // xy = tiling, zw = offset
} pico;

vec2 PicoTransformUv(vec2 uv, vec4 mapST)
{
    return uv * mapST.xy + mapST.zw;
}

#endif // PICO_COMMON_GLSL
