#ifndef GEOMETRY_COMMON_GLSL_INCLUDED
#define GEOMETRY_COMMON_GLSL_INCLUDED

#include stack.glsl

uniform sampler2D raysMap;
uniform float3 geoPosition;
uniform float4x4 geoGeoWorldMat;
uniform float4x4 geoWorldGeoMat;

float32 unionDistances(float32 d1, float32 d2)
{
  return min(d1, d2);
}

float32 intersectDistances(float32 d1, float32 d2)
{
  return max(d1, d2);
}

float32 subtractDistances(float32 d1, float32 d2)
{
  return max(d1, -d2);
}

#endif
