#include <algorithm>

#include <assets/assets_factory.h>
#include <../bin/shaders/declarations.h>

#include "memory_manager.h"

#include "scene.h"

using std::vector;
using std::string;

struct Scene
{
  AssetPtr geometryRoot;
  vector<AssetPtr> lightSources;

  string name;
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
  (*outScene)->name = "default_scene";

  return TRUE;
}

void destroyScene(Scene* scene)
{
  scene->geometryRoot = AssetPtr(nullptr);
  scene->lightSources.clear();
  
  engineFreeObject(scene, MEMORY_TYPE_GENERAL);
}

bool8 serializeScene(Scene* scene, nlohmann::json& json)
{
  json["name"] = scene->name;
  assetSerialize(scene->geometryRoot, json["geometry_root"]);

  json["lights_count"] = scene->lightSources.size();
  for(uint32 i = 0; i < scene->lightSources.size(); i++)
  {
    assetSerialize(scene->lightSources[i], json["lights"][i]);
  }

  return TRUE;
}

bool8 deserializeScene(Scene* scene, nlohmann::json& json)
{
  scene->lightSources.clear();
  
  scene->name = json["name"];
  scene->geometryRoot = createAssetFromJson(json["geometry_root"]);

  uint32 lightsCount = json["lights_count"];
  for(uint32 i = 0; i < lightsCount; i++)
  {
    scene->lightSources.push_back(createAssetFromJson(json["lights"][i]));
  }

  return TRUE;
}

void updateScene(Scene* scene, float64 delta)
{
  geometryUpdate(scene->geometryRoot, delta);
}

void sceneSetName(Scene* scene, const std::string& name)
{
  scene->name = name;
}

const std::string& sceneGetName(Scene* scene)
{
  return scene->name;
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

std::vector<AssetPtr> sceneGetEnabledLightSources(Scene* scene)
{
  std::vector<AssetPtr> result;
  for(AssetPtr lsource: scene->lightSources)
  {
    if(lightSourceIsEnabled(lsource) == TRUE)
    {
      result.push_back(lsource);
    }
  }

  return result;
}
