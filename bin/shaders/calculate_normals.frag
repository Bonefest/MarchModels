#version 450 core

#include common.glsl

layout(location = 0) out float3 outNormal;

layout(location = 0) uniform sampler2D distancesMap;
layout(location = 1) uniform usampler2D idsMap;

// NOTE: Convert fragCoord into ray in camera's direction, multiply ray by
// distance, traversed along that ray - the result is a point in camera's space
float3 convertFragCoordToCameraSpace(float2 fragCoord)
{
  float3 cameraRay = generateRayDir(fragCoordToUV(fragCoord));
  float distance = texelFetch(distancesMap, int2(fragCoord.xy), 0).r;

  return cameraRay * distance;
}

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);
    uint32 id = texelFetch(idsMap, ifragCoord, 0).r;

    if(id == UKNOWN_GEOMETRY_ID)
    {
      outNormal = float3(0.0, 0.0, 0.0);
    }
    else
    {
      float3 centerPos = convertFragCoordToCameraSpace(gl_FragCoord.xy);
      float3 topPos    = convertFragCoordToCameraSpace(gl_FragCoord.xy + float2(0, 1));
      float3 rightPos  = convertFragCoordToCameraSpace(gl_FragCoord.xy + float2(1, 0));

      float3 cameraSpaceN = normalize(cross(rightPos - centerPos, topPos - centerPos));
      
      outNormal = normalize((params.camCameraWorldMat * float4(cameraSpaceN, 0.0)).xyz);
    }
}
