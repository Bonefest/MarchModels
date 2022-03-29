#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "ptr.h"
#include "defines.h"

using AssetType = uint32;

struct Asset;

struct AssetInterface
{
  void (*destroy)(Asset*);
  
  bool8(*serialize)(Asset*, nlohmann::json&);
  bool8(*deserialize)(Asset*, nlohmann::json&);
  uint32(*getSize)(Asset*);
  
  void (*onNameChanged)(Asset*, const std::string&, const std::string&);
  
  AssetType type;
};

ENGINE_API bool8 allocateAsset(AssetInterface interface, const std::string& name, Asset** outAsset);
ENGINE_API void destroyAsset(Asset* asset);

ENGINE_API bool8 assetSerialize(Asset* asset, nlohmann::json& jsonData);
ENGINE_API bool8 assetDeserialize(Asset* asset, nlohmann::json& jsonData);

// WARNING: Be careful with name changing. When an asset from asset manager is changing a name,
// asset manger should be informed immediately!
// CONSIDER: We can add a previousName variable, so that asset manager can check name changing
// by itsel
// CONSIDER: We can also trigger an event
ENGINE_API void assetSetName(Asset* asset, const std::string& newName);
ENGINE_API const std::string& assetGetName(Asset* asset);
ENGINE_API AssetType assetGetType(Asset* asset);

void assetSetInternalData(Asset* asset, void* data);
void* assetGetInternalData(Asset* asset);

using AssetPtr = SharedPtr<Asset, destroyAsset>;
using AssetWPtr = WeakPtr<Asset, destroyAsset>;
