#version 450 core

#include common.glsl

layout(location = 0) in float3 gridVertexPos;

void main()
{
  gl_Position = params.camWorldNDCMat * float4(gridVertexPos, 1.0f);
  gl_Position.x = -gl_Position.x;
}
