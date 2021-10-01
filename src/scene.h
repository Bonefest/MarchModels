#pragma once

#include <vector>

#include "geometry.h"

struct LightSource;

struct Scene;

struct IntersectionDetails
{
  Geometry* geometry;
  float3 normal;
  float32 totalDistance;
  bool8 intersected;
};

ENGINE_API bool8 createScene(Scene** outScene);
ENGINE_API void destroyScene(Scene* scene);

ENGINE_API void sceneAddGeometry(Scene* scene, Geometry* geometry);
ENGINE_API bool8 sceneRemoveGeometry(Scene* scene, Geometry* geometry);
ENGINE_API const std::vector<Geometry*>& sceneGetGeometry(Scene* scene);

ENGINE_API void sceneAddLightSource(Scene* scene, LightSource* lightSource);
ENGINE_API void sceneRemoveLightSource(Scene* scene, LightSource* lightSource);
ENGINE_API const std::vector<LightSource*>& sceneGetLightSources(Scene* scene);

ENGINE_API IntersectionDetails sceneFindIntersection(Scene* scene, Ray ray, bool8 calculateNormal = TRUE);
