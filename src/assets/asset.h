#pragma once

#include <string>

#include "defines.h"

using AssetType = uint32;

struct Asset;

struct AssetInterface
{
  void (*destroy)(Asset*);
  
  bool8(*serialize)(Asset*);
  bool8(*deserialize)(Asset*);
  uint32(*getSize)(Asset*);
  
  AssetType type;
};

ENGINE_API bool8 allocateAsset(AssetInterface interface, const std::string& name, Asset** outAsset);
ENGINE_API void destroyAsset(Asset* asset);

ENGINE_API bool8 assetSerialize();
ENGINE_API bool8 assetDeserialize();

ENGINE_API const std::string& assetGetName(Asset* asset);
ENGINE_API AssetType assetGetType(Asset* asset);

void assetSetInternalData(Asset* asset, void* data);
void* assetGetInternalData(Asset* asset);
