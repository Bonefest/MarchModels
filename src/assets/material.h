#pragma once

#include "asset.h"

static const AssetType ASSET_TYPE_MATERIAL = 0x89c5e3b6;

ENGINE_API bool8 createMaterial(const std::string& name, Asset** lsource);
