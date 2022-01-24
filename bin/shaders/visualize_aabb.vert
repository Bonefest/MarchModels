#version 450 core

#include defines.glsl

// Per-vetex attributes
layout(location = 0) in float3 vertexPos;

// Per-instance attributes
layout(location = 1) in float3 aabbPosition;
layout(location = 2) in float3 aabbSize;

layout(location = 0) uniform mat4 viewProj;

void main()
{
  float4 pos = viewProj * float4(vertexPos * aabbSize + aabbPosition, 1.0);
  gl_Position = float4(-pos.x, pos.y, pos.z, pos.w);
}

