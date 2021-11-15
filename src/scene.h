#pragma once

#include <vector>

#include "assets/geometry.h"

struct LightSource;

struct Scene;

struct IntersectionDetails
{
  // TODO: Provide AssetPtr instead of raw pointer
  Asset* geometry;
  float3 normal;
  float32 totalDistance;
  bool8 intersected;
};

ENGINE_API bool8 createScene(Scene** outScene);
ENGINE_API void destroyScene(Scene* scene);

ENGINE_API void sceneAddGeometry(Scene* scene, AssetPtr geometry);
ENGINE_API bool8 sceneRemoveGeometry(Scene* scene, AssetPtr geometry);
ENGINE_API std::vector<AssetPtr>& sceneGetGeometry(Scene* scene);

ENGINE_API void sceneAddLightSource(Scene* scene, LightSource* lightSource);
ENGINE_API void sceneRemoveLightSource(Scene* scene, LightSource* lightSource);
ENGINE_API std::vector<LightSource*>& sceneGetLightSources(Scene* scene);

ENGINE_API IntersectionDetails sceneFindIntersection(Scene* scene, Ray ray, bool8 calculateNormal = TRUE);
