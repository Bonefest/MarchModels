#ifndef GEOMETRY_COMMON_GLSL_INCLUDED
#define GEOMETRY_COMMON_GLSL_INCLUDED

#include stack.glsl

layout(location = 0) uniform sampler2D raysMap;
#if SHADOW_PATH
  layout(location = 1) uniform sampler2D depthMap;
#endif

layout(std140, binding = GEOMETRY_TRANSFORMS_UBO_BINDING) uniform GeometryTransformParametersUBO
{
  GeometryTransformParameters geo;
};

GeometryData unionGeometries(GeometryData geometry1, GeometryData geometry2)
{
  return geometry1.distance < geometry2.distance ? geometry1 : geometry2;
}

GeometryData intersectGeometries(GeometryData geometry1, GeometryData geometry2)
{
  return createGeometryData(max(geometry1.distance, geometry2.distance), geometry1.id);
}

GeometryData subtractGeometries(GeometryData geometry1, GeometryData geometry2)
{
  return createGeometryData(max(geometry1.distance, -geometry2.distance), geometry1.id);  
}

#endif
