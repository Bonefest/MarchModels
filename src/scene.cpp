#include <algorithm>

#include "memory_manager.h"

#include "scene.h"

using std::vector;

struct Scene
{
  vector<AssetPtr> geometryArray;
  vector<LightSource*> lightSourceArray;
};

bool8 createScene(Scene** outScene)
{
  *outScene = engineAllocObject<Scene>(MEMORY_TYPE_GENERAL);

  return TRUE;
}

void destroyScene(Scene* scene)
{
  engineFreeObject(scene, MEMORY_TYPE_GENERAL);
}

void sceneAddGeometry(Scene* scene, AssetPtr geometry)
{
  scene->geometryArray.push_back(geometry);
}

bool8 sceneRemoveGeometry(Scene* scene, AssetPtr geometry)
{
  auto it = std::find(scene->geometryArray.begin(), scene->geometryArray.end(), geometry);
  if(it == scene->geometryArray.end())
  {
    return FALSE;
  }

  scene->geometryArray.erase(it);
  
  return TRUE;
}

std::vector<AssetPtr>& sceneGetGeometry(Scene* scene)
{
  return scene->geometryArray;
}

void sceneAddLightSource(Scene* scene, LightSource* lightSource)
{

}

void sceneRemoveLightSource(Scene* scene, LightSource* lightSource)
{

}

std::vector<LightSource*>& sceneGetLightSources(Scene* scene)
{
  return scene->lightSourceArray;
}

IntersectionDetails sceneFindIntersection(Scene* scene, Ray ray, bool8 calculateNormal)
{
  // TODO: Load these data from configuration system
  const static float32 MIN_SDF_VALUE = 0.01f;
  const static uint32 MAX_STEPS = 8;
  
  Ray localRay = ray;

  IntersectionDetails details = {};

  uint32 step;
  for(step = 0; step < MAX_STEPS; step++)
  {
    float32 minDistance = std::numeric_limits<float32>::max();
    Asset* closestGeometry = nullptr;
    
    for(AssetPtr geometry: scene->geometryArray)
    {
      Asset* closestLeaf = nullptr;
      float32 distance = geometryCalculateDistanceToPoint(geometry, localRay.origin, &closestLeaf);
      if(distance < minDistance)
      {
        minDistance = distance;
        closestGeometry = closestLeaf;
      }
    }

    if(minDistance < MIN_SDF_VALUE)
    {
      details.geometry = closestGeometry;
      if(calculateNormal)
      {
        details.normal = geometryCalculateNormal(closestGeometry, ray.origin);
      }

      break;
    }
    else
    {
      details.totalDistance += minDistance;
      localRay.origin += localRay.direction * minDistance;
    }
  }

  details.intersected = step >= MAX_STEPS ? FALSE : TRUE;

  return details;
}

