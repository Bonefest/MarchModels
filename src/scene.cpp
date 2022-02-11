#include <algorithm>

#include <../bin/shaders/declarations.h>

#include "memory_manager.h"

#include "scene.h"

using std::vector;

struct Scene
{
  AssetPtr geometryRoot;
  vector<AssetPtr> lightSources;
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

  return TRUE;
}

void destroyScene(Scene* scene)
{
  engineFreeObject(scene, MEMORY_TYPE_GENERAL);
}

void updateScene(Scene* scene, float64 delta)
{
  geometryUpdate(scene->geometryRoot, delta);
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

const std::set<AssetPtr>& sceneGetAllChildren(Scene* scene)
{
  return geometryRootGetAllChildren(scene->geometryRoot);
}

bool8 sceneAddLightSource(Scene* scene, AssetPtr lightSource)
{
  if(scene->lightSources.size() >= MAX_LIGHT_SOURCES_COUNT)
  {
    return FALSE;
  }
  
  scene->lightSources.push_back(lightSource);
  return TRUE;
}

bool8 sceneRemoveLightSource(Scene* scene, AssetPtr lightSource)
{
  auto it = std::find(scene->lightSources.begin(),
                      scene->lightSources.end(),
                      lightSource);

  if(it == scene->lightSources.end())
  {
    return FALSE;
  }

  scene->lightSources.erase(it);
  return TRUE;
}

std::vector<AssetPtr>& sceneGetLightSources(Scene* scene)
{
  return scene->lightSources;
}

