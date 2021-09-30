#pragma once

#include "shape.h"

struct LightSource;

struct Scene
{

};

struct IntersectionDetails
{
  Shape* shape;
  // float3 normal;
  float distance;
};

bool8 createScene(Scene** outScene);
void destroyScene(Scene* scene);

void sceneAddShape(Scene* scene, Shape* shape);
void sceneRemoveShape(Scene* scene, Shape* shape);
const std::vector<Shape*>& sceneGetShapes(Scene* scene);

void sceneAddLightSource(Scene* scene, LightSource* lightSource);
void sceneRemoveLightSource(Scene* scene, LightSource* lightSource);
const std::vector<LightSource*>& sceneGetLightSources(Scene* scene);

IntersectionDetails sceneFindIntersection(Scene* scene, Ray ray);
