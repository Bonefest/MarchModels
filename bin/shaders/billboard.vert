#version 450 core

#include common.glsl

// Per-vetex attributes
layout(location = 0) in float3 rectVertexPos;
layout(location = 1) in float2 rectUV;

// Per-instance attributes
layout(location = 2) in float2 billboardOffset;
layout(location = 3) in float2 billboardScale;
layout(location = 4) in float3 billboardPosition;
layout(location = 5) in float4 billboardColor;
layout(location = 6) in float4 billboardUVRect;

// Output attributes
layout(location = 0) out float4 outBillboardColor;
layout(location = 1) out float2 outUV;

void main()
{
  outBillboardColor = billboardColor;
  
  float2 uvSize = billboardUVRect.zw - billboardUVRect.xy;
  outUV = rectUV * uvSize + billboardUVRect.xy;

  // NOTE: Convert center of rectangle in NDC to Camera Space
  float4 posOffsetCS = params.camNDCCameraMat * float4(billboardOffset, 0.0f, 0.0f);
  
  float4 billboardPosCS = params.camWorldCameraMat * float4(billboardPosition, 1.0);
  billboardPosCS.xy += posOffsetCS.xy;

  gl_Position = params.camCameraNDCMat * float4(rectVertexPos * float3(billboardScale, 1.0) + billboardPosCS.xyz, 1.0);
  gl_Position.x = -gl_Position.x;
}

