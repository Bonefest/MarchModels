#include <memory_manager.h>

#include "asset.h"

using std::string;

struct Asset
{
  AssetInterface interface;
  string name;

  void* internalData;
};

bool8 allocateAsset(AssetInterface interface, const string& name, Asset** outAsset)
{
  *outAsset = engineAllocObject<Asset>(MEMORY_TYPE_GENERAL);
  Asset* asset = *outAsset;
  asset->interface = interface;
  asset->name = name;

  return TRUE;
}

void destroyAsset(Asset* asset)
{
  if(asset->interface.destroy != nullptr)
  {
    asset->interface.destroy(asset);
  }

  engineFreeObject(asset, MEMORY_TYPE_GENERAL);
}

const std::string& assetGetName(Asset* asset)
{
  return asset->name;
}

AssetType assetGetType(Asset* asset)
{
  return asset->interface.type;
}

void assetSetInternalData(Asset* asset, void* data)
{
  asset->internalData = data;
}

void* assetGetInternalData(Asset* asset)
{
  return asset->internalData;
}

