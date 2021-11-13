#pragma once

#include <vector>

#include "asset.h"

ENGINE_API bool8 initAssetsManager();
ENGINE_API void shutdownAssetsManager();

ENGINE_API bool8 assetsManagerAddAsset(AssetPtr asset);
ENGINE_API bool8 assetsManagerRemoveAsset(AssetPtr asset);
ENGINE_API bool8 assetsManagerRemoveAsset(const std::string& name);

ENGINE_API AssetPtr assetsManagerFindAsset(const std::string& name);
ENGINE_API bool8 assetsManagerHasAsset(const std::string& name);

ENGINE_API bool8 assetsManagerLoadFromFile(const std::string& fileName);

ENGINE_API const std::vector<AssetPtr>& assetsManagerGetAssets();
ENGINE_API std::vector<AssetPtr> assetsManagerGetAssetsByType(AssetType type);

