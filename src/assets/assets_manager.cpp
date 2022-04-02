#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <unordered_map>

#include <nlohmann/json.hpp>

using std::string;
using std::vector;
using std::unordered_map;
using namespace nlohmann;

#include <logging.h>
#include <scheduler.h>
#include <assets/assets_factory.h>

#include "assets_manager.h"

struct AssetsManager
{
  vector<AssetPtr> assets;
};

static AssetsManager manager;


static void assetsManagerSaveToFile(float32 time, const std::string& fileName, void* owner, void* data)
{
  json jsonData;
  for(uint32 i = 0; i < manager.assets.size(); i++)
  {
    assetSerialize(manager.assets[i], jsonData["assets"][i]);
  }
  
  jsonData.dump(4);
  
  std::ofstream file(fileName);
  file << jsonData;
  
  LOG_INFO("Assets have been successfully saved!");
}

bool8 initAssetsManager()
{
  if(assetsManagerLoadFromFile("saved_assets.json") == FALSE)
  {
    LOG_WARNING("No saved assets were discovered, either they were deleted or moved.");
  }

  schedulerRegisterFunctionT1(assetsManagerSaveToFile, 30.0f, "saved_assets.json");
  
  return TRUE;
}

void shutdownAssetsManager()
{
  manager.assets.clear();
}

bool8 assetsManagerLoadFromFile(const std::string& fileName)
{
  std::ifstream file(fileName);
  if(file.is_open())
  {
    json jsonData;
    file >> jsonData;

    for(json assetJson: jsonData["assets"])
    {
      AssetPtr asset = createAssetFromJson(assetJson);
      if(asset == AssetPtr(nullptr))
      {
        LOG_ERROR("Asset manager cannot load an asset from '%s'!", fileName.c_str());
      }
      else
      {
        if(assetsManagerAddAsset(asset) == FALSE)
        {
          LOG_WARNING("File '%s' contains asset '%s', which is already registered!",
                      fileName.c_str(), assetJson.at("name").get<std::string>().c_str());
        }
      }
    }

    return TRUE;
  }

  return FALSE;
}

bool8 assetsManagerAddAsset(AssetPtr asset)
{
  auto assetIt = std::find_if(manager.assets.begin(),
                              manager.assets.end(),
                              [asset](AssetPtr arrAsset){ return assetGetName(asset) == assetGetName(arrAsset); });
  
  if(assetIt != manager.assets.end())
  {
    return FALSE;
  }

  manager.assets.push_back(asset);
  return TRUE;
}

bool8 assetsManagerRemoveAsset(Asset* asset)
{
  auto assetIt = std::find(manager.assets.begin(), manager.assets.end(), asset);
  if(assetIt != manager.assets.end())
  {
    manager.assets.erase(assetIt);
    return TRUE;
  }

  return FALSE;
}

bool8 assetsManagerRemoveAsset(const std::string& name)
{
  auto assetIt = std::find_if(manager.assets.begin(),
                              manager.assets.end(),
                              [&name](Asset* asset) { return assetGetName(asset) == name; });

  if(assetIt != manager.assets.end())
  {
    manager.assets.erase(assetIt);
    return TRUE;
  }

  return FALSE;
}

AssetPtr assetsManagerFindAsset(const std::string& name)
{
  auto assetIt = std::find_if(manager.assets.begin(),
                              manager.assets.end(),
                              [&name](AssetPtr asset){ return assetGetName(asset) == name; });

  if(assetIt != manager.assets.end())
  {
    return *assetIt;
  }
  
  return AssetPtr(nullptr);
}

bool8 assetsManagerHasAsset(AssetPtr asset)
{
  auto assetIt = std::find_if(manager.assets.begin(),
                              manager.assets.end(),
                              [&asset](AssetPtr lasset) { return lasset == asset; });
  
  return assetIt != manager.assets.end() ? TRUE : FALSE;
}

bool8 assetsManagerHasAsset(const std::string& name)
{
  return assetsManagerFindAsset(name) != nullptr;
}

const std::vector<AssetPtr>& assetsManagerGetAssets()
{
  return manager.assets;
}

std::vector<AssetPtr> assetsManagerGetAssetsByType(AssetType type)
{
  std::vector<AssetPtr> result;
  for(auto it = manager.assets.begin(); it != manager.assets.end(); it++)
  {
    if(assetGetType(*it) == type)
    {
      result.push_back(*it);
    }
  }

  return result;
}
