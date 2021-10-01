#include <algorithm>

#include "memory_manager.h"

#include "scene.h"

using std::vector;

struct Scene
{
  vector<Geometry*> geometryArray;
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

void sceneAddGeometry(Scene* scene, Geometry* geometry)
{
  scene->geometryArray.push_back(geometry);
}

bool8 sceneRemoveGeometry(Scene* scene, Geometry* geometry)
{
  auto it = std::find(scene->geometryArray.begin(), scene->geometryArray.end(), geometry);
  if(it == scene->geometryArray.end())
  {
    return FALSE;
  }

  scene->geometryArray.erase(it);
  
  return TRUE;
}

const vector<Geometry*>& sceneGetGeometrys(Scene* scene)
{
  return scene->geometryArray;
}

void sceneAddLightSource(Scene* scene, LightSource* lightSource)
{

}

void sceneRemoveLightSource(Scene* scene, LightSource* lightSource)
{

}

// const std::vector<LightSource*>& sceneGetLightSources(Scene* scene)
// {

// }

IntersectionDetails sceneFindIntersection(Scene* scene, Ray ray)
{

}

