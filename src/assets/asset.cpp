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

bool8 assetSerialize(Asset* asset, nlohmann::json& jsonData)
{
  if(asset->interface.deserialize != nullptr)
  {
    jsonData["name"] = asset->name;

    return asset->interface.deserialize(asset, jsonData);
  }

  return FALSE;
}

bool8 assetDeserialize(Asset* asset, nlohmann::json& jsonData)
{
  if(asset->interface.serialize != nullptr)
  {
    asset->name = jsonData["name"];
    
    return asset->interface.serialize(asset, jsonData);
  }

  return FALSE;  
}

void assetSetName(Asset* asset, const std::string& newName)
{
  std::string prevName = asset->name;
  
  asset->name = newName;
  if(asset->interface.onNameChanged != nullptr)
  {
    asset->interface.onNameChanged(asset, prevName, newName);
  }
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

