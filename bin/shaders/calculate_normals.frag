#version 450 core

#include common.glsl

layout(location = 0) out float3 outNormal;

layout(location = 0) uniform sampler2D distancesMap;
layout(location = 1) uniform usampler2D idsMap;

// NOTE: Convert fragCoord into ray in world space, multiply ray by
// distance, traversed along that ray - the result is a point on a surface in world space
float3 convertFragCoordToWorldSpace(float2 fragCoord)
{
  float3 worldRay = generateRayDir(fragCoordToUV(fragCoord));
  float distance = texelFetch(distancesMap, int2(fragCoord.xy), 0).r;

  return normalize(worldRay) * distance + params.camPosition.xyz;
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
      float3 centerPos = convertFragCoordToWorldSpace(gl_FragCoord.xy);
      float3 topPos    = convertFragCoordToWorldSpace(gl_FragCoord.xy + float2(0, 1));
      float3 rightPos  = convertFragCoordToWorldSpace(gl_FragCoord.xy + float2(1, 0));

      outNormal = normalize(cross(rightPos - centerPos, topPos - centerPos));
    }
}
