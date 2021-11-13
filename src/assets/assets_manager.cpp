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
  vector<AssetPtr> assets;
};

static AssetsManager manager;

bool8 initAssetsManager()
{
  return TRUE;
}

void shutdownAssetsManager()
{
  for(Asset* asset: manager.assets)
  {
    destroyAsset(asset);
  }
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
  
  return nullptr;
}

bool8 assetsManagerHasAsset(AssetPtr asset)
{
  auto assetIt = std::find_if(manager.assets.begin(),
                              manager.assets.end(),
                              [&asset](AssetPtr lasset) { return lasset.ptr == asset.ptr; });
  
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
