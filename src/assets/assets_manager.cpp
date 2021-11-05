#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>


using std::string;
using std::vector;
using std::unordered_map;

#include "assets_manager.h"

struct AssetsManager
{
  vector<Asset*> assets;
};

static AssetsManager manager;

bool8 initAssetsManager()
{
  return TRUE;
}

void shutdownAssetsManager()
{

}

bool8 assetsManagerAddAsset(Asset* asset)
{
  auto assetIt = std::find_if(manager.assets.begin(),
                              manager.assets.end(),
                              [asset](Asset* arrAsset){ return assetGetName(asset) == assetGetName(arrAsset); });
  
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

Asset* assetsManagerFindAsset(const std::string& name)
{
  auto assetIt = std::find_if(manager.assets.begin(),
                              manager.assets.end(),
                              [&name](Asset* asset){ return assetGetName(asset) == name; });

  if(assetIt != manager.assets.end())
  {
    return *assetIt;
  }
  
  return nullptr;
}

bool8 assetsManagerHasAsset(const std::string& name)
{
  return assetsManagerFindAsset(name) != nullptr;
}

const std::vector<Asset*>& assetsManagerGetAssets()
{
  return manager.assets;
}

std::vector<Asset*> assetsManagerGetAssetsByType(AssetType type)
{
  std::vector<Asset*> result;
  for(auto it = manager.assets.begin(); it != manager.assets.end(); it++)
  {
    if(assetGetType(*it) == type)
    {
      result.push_back(*it);
    }
  }

  return result;
}
