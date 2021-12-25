#version 450 core

#include common.glsl

layout(location = 0) out float3 outColor;
layout(location = 0) uniform sampler2D LDRMap;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);
   
    outColor = pow(texelFetch(LDRMap, ifragCoord, 0).rgb, params.invGamma.rrr);
}
