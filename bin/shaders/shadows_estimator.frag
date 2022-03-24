#version 450 core
#extension GL_ARB_shader_stencil_export : require

#include stack.glsl

out float4 outShadows;

layout(location = 0) uniform uint32 lightIndex;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);
    float32 t = stackGetTotalDistance(ifragCoord);
    float32 h = stackEmpty(ifragCoord) ? INF_DISTANCE : stackFront(ifragCoord).distance;
    float32 k = lightParams[lightIndex].shadowFactor;

    // NOTE: If distance to the nearest surface is smaller than intersection
    // distance, then texel is in full shadow
    float32 shadow = t < INF_DISTANCE ? mix(k * h / t, 0.0, h < INT_DISTANCE) : 1.0f;

    // NOTE: Init values to 1, so that during blending, min(...) operation won't
    // affect unused channels
    float4 result = 1.0f.xxxx;

    // NOTE: Change only channel corresponding to the current light index
    result[lightIndex % 4] = shadow;

    outShadows = result;
}
