#include <algorithm>

#include "memory_manager.h"

#include "scene.h"

using std::vector;

struct Scene
{
  AssetPtr geometryRoot;
  vector<LightSource*> lightSourceArray;
};

bool8 createScene(Scene** outScene)
{
  *outScene = engineAllocObject<Scene>(MEMORY_TYPE_GENERAL);

  Asset* sceneRootGeometry = nullptr;
  createGeometry("scene_root", &sceneRootGeometry);

  if(sceneRootGeometry == nullptr)
  {
    return FALSE;
  }

  (*outScene)->geometryRoot = AssetPtr(sceneRootGeometry);
  geometrySetCombinationFunction((*outScene)->geometryRoot, COMBINATION_UNION);

  return TRUE;
}

void destroyScene(Scene* scene)
{
  engineFreeObject(scene, MEMORY_TYPE_GENERAL);
}

void sceneAddGeometry(Scene* scene, AssetPtr geometry)
{
  geometryAddChild(scene->geometryRoot, geometry);
}

bool8 sceneRemoveGeometry(Scene* scene, AssetPtr geometry)
{
  return geometryRemoveChild(scene->geometryRoot, geometry);
}

std::vector<AssetPtr>& sceneGetChildren(Scene* scene)
{
  return geometryGetChildren(scene->geometryRoot);
}

AssetPtr sceneGetGeometryRoot(Scene* scene)
{
  return scene->geometryRoot;
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

