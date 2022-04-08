#version 450 core

#include common.glsl

// Per-instance attributes
layout(location = 0) in float3 axisColor;
layout(location = 1) in float4x4 axisMat;

// Output attributes
layout(location = 0) out float3 outAxisColor;

void main()
{
  outAxisColor = axisColor;

  float4 vertexPos = float4(0.0f, 0.0f, gl_VertexID, 1.0f);

  // NOTE: NDC is in LHS --> We can apply our usual transformations
  gl_Position = params.camWorldNDCMat * axisMat * vertexPos;

  // NOTE: The only thing, is that we need to inverse the x (in OpenGL positive
  // x is on the right, in our system positive x is on the left)
  gl_Position.x = -gl_Position.x;
}
