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

  // NOTE:
  //   1) Perform aspect ratio adjustment in order to save shape's form
  //   2) Additionally scale position by given scale
  //   3) Additionally offset position by given offset
  //   4) Convert given rectangle vertex position in NDC into Camera Space
  
  float32 invAspect = float32(params.gapResolution.y) / float32(params.gapResolution.x);
  float3 vertexPos = rectVertexPos * float3(billboardScale * float2(invAspect, 1.0), 1.0) + float3(billboardOffset * billboardScale, 0.0);
  float4 vertexPosCS = params.camNDCCameraMat * float4(vertexPos, 1.0);
  vertexPosCS /= vertexPosCS.w;

  // NOTE: Convert billboard center pos from Wolrd Space to Camera Space 
  float4 billboardPosCS = params.camWorldCameraMat * float4(billboardPosition, 1.0);

  // NOTE: Finally, offset billboard's vertex by billboard position and convert
  // the result into NDC space
  gl_Position = params.camCameraNDCMat * float4(vertexPosCS.xyz + billboardPosCS.xyz, 1.0);
  gl_Position.x = -gl_Position.x;
}

