#version 450 core

#include common.glsl

layout(location = 0) out float3 outRadiance;

layout(location = 0) uniform float3 bottomBGColor;
layout(location = 1) uniform float3 topBGColor;

void main()
{
  float2 uv = fragCoordToUV(gl_FragCoord.xy);
  float3 worldRay = generateRayDir(uv);
  float32 elevationAngle = dot(worldRay, float3(0.0f, 1.0f, 0.0f)) * 0.5 + 0.5;
  outRadiance = pow(mix(bottomBGColor, topBGColor, elevationAngle), params.invGamma.xxx);  
}
