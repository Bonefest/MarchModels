#version 450 core

#include defines.glsl

// Per-vetex attributes
layout(location = 0) in float3 rectVertexPos;
layout(location = 1) in float2 rectUV;

// Per-instance attributes
layout(location = 2) in float2 billboardSize;
layout(location = 3) in float3 billboardPosition;
layout(location = 4) in float4 billboardColor;
layout(location = 5) in float4 billboardUVRect;

// Output attributes
layout(location = 0) out float4 outBillboardColor;
layout(location = 1) out float2 outUV;

// Uniforms
layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;

void main()
{
  outBillboardColor = billboardColor;

  float2 uvSize = billboardUVRect.zw - billboardUVRect.xy;
  outUV = rectUV * uvSize + billboardUVRect.xy;
  
  float4 billboardPosCS = view * float4(billboardPosition, 1.0);

  gl_Position = proj * float4(rectVertexPos * float3(billboardSize, 1.0) + billboardPosCS.xyz, 1.0);
  gl_Position.x = -gl_Position.x;
}

