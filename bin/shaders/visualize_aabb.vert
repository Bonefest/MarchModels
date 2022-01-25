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
  // NOTE: NDC is in LHS --> We can apply our usual transformations
  gl_Position = viewProj * float4(vertexPos.x, vertexPos.yz, 1.0);

  // NOTE: The only thing, is that we need to inverse the x (in OpenGL positive
  // x is on the right, in our system positive x is on the left)
  gl_Position.x = -gl_Position.x;
}

