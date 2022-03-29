#pragma once

#include <nlohmann/json.hpp>
#include "asset.h"

ENGINE_API AssetPtr createAssetFromJson(nlohmann::json& jsonData);
