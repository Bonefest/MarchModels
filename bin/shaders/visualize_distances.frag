#version 450 core

#include defines.glsl


layout(location = 0) out float3 outDistanceColor;

layout(location = 0) uniform sampler2D distancesMap;
layout(location = 1) uniform float2 distancesRange;
layout(location = 2) uniform float3 closestColor;
layout(location = 3) uniform float3 farthestColor;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

    float32 distance = clamp(texelFetch(distancesMap, ifragCoord, 0).r, distancesRange.x, distancesRange.y);
    float32 t = (distance - distancesRange.x) / (distancesRange.y - distancesRange.x);

    outDistanceColor = mix(closestColor, farthestColor, t);
}
