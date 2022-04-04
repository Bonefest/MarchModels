#version 450 core

#include common.glsl

layout(location = 0) out float4 outFogColor;

layout(location = 0) uniform sampler2D depthMap;
layout(location = 1) uniform float2 fogMinMax;
layout(location = 2) uniform float2 fogNearFar;
layout(location = 3) uniform float3 fogColor;

void main()
{
    const float32 near = fogNearFar.x;
    const float32 far = fogNearFar.y;

    const float32 fogMin = fogMinMax.x;
    const float32 fogMax = fogMinMax.y;

    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);
    float2 uv = fragCoordToUV(gl_FragCoord.xy);
    float3 worldPos = getWorldPos(uv, ifragCoord, depthMap);
    float3 cameraPos = params.camPosition.xyz;

    float32 distanceToCam = distance(worldPos, cameraPos);
    float32 fogIntensity = clamp((distanceToCam - near) / (far - near),
                                 fogMin, fogMax);

    outFogColor = float4(fogColor, fogIntensity);
}
