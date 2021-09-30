#pragma once

#include <vector>

#include "shape.h"

struct LightSource;

struct Scene;

struct IntersectionDetails
{
  Shape* shape;
  // float3 normal;
  float distance;
};

ENGINE_API bool8 createScene(Scene** outScene);
ENGINE_API void destroyScene(Scene* scene);

ENGINE_API void sceneAddShape(Scene* scene, Shape* shape);
ENGINE_API bool8 sceneRemoveShape(Scene* scene, Shape* shape);
ENGINE_API const std::vector<Shape*>& sceneGetShapes(Scene* scene);

ENGINE_API void sceneAddLightSource(Scene* scene, LightSource* lightSource);
ENGINE_API void sceneRemoveLightSource(Scene* scene, LightSource* lightSource);
ENGINE_API const std::vector<LightSource*>& sceneGetLightSources(Scene* scene);

ENGINE_API IntersectionDetails sceneFindIntersection(Scene* scene, Ray ray);
