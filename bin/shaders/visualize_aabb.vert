#version 450 core

#include common.glsl

// Per-vetex attributes
layout(location = 0) in float3 vertexPos;

// Per-instance attributes
layout(location = 1) in float3 aabbPosition;
layout(location = 2) in float3 aabbSize;
layout(location = 3) in float3 aabbColor;

// Output attributes
layout(location = 0) out float3 outAABBColor;

void main()
{
  outAABBColor = aabbColor;

  // NOTE: NDC is in LHS --> We can apply our usual transformations
  gl_Position = params.camWorldNDCMat * float4(vertexPos * aabbSize + aabbPosition, 1.0);

  // NOTE: The only thing, is that we need to inverse the x (in OpenGL positive
  // x is on the right, in our system positive x is on the left)
  gl_Position.x = -gl_Position.x;
}

