#version 450 core

#include common.glsl

layout(location = 0) out float3 outNormalColor;

layout(location = 0) uniform sampler2D normalsMap;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

    // NOTE: Convert values into linear space
    outNormalColor = gammaDecode(texelFetch(normalsMap, ifragCoord, 0).rgb * 0.5 + 0.5);
}
