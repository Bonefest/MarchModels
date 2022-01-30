#version 450 core

#include defines.glsl

layout(location = 0) in float3 vertexPos;
layout(location = 0) uniform mat4 viewProj;

void main()
{
  gl_Position = viewProj * float4(vertexPos, 1.0);
  gl_Position.x = -gl_Position.x;
}

