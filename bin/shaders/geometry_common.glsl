#ifndef GEOMETRY_COMMON_GLSL_INCLUDED
#define GEOMETRY_COMMON_GLSL_INCLUDED

#include stack.glsl

layout(location = 0) uniform sampler2D raysMap;

layout(std140, binding = GEOMETRY_TRANSFORMS_UBO_BINDING) uniform GeometryTransformParametersUBO
{
  GeometryTransformParameters geo;
};


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
