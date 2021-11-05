#pragma once

#include <string>
#include <unordered_map>

#include "asset.h"
#include "maths/common.h"

static const AssetType ASSET_TYPE_SCRIPT_FUNCTION = 0xdf426230;

using ScriptFunctionArgs = std::unordered_map<std::string, float32>;

enum ScriptFunctionType
{
  SCRIPT_FUNCTION_TYPE_SDF,
  SCRIPT_FUNCTION_TYPE_IDF,
  SCRIPT_FUNCTION_TYPE_ODF,

  SCRIPT_FUNCTION_TYPE_COUNT
};

ENGINE_API bool8 createScriptFunction(ScriptFunctionType type,
                                      const std::string& name,
                                      Asset** outAsset);

ENGINE_API void scriptFunctionSetArgValue(Asset* asset, const std::string& argName, float32 value);
ENGINE_API float32 scriptFunctionGetArgValue(Asset* asset, const std::string& argName);
ENGINE_API ScriptFunctionArgs& scriptFunctionGetArgs(Asset* asset);
ENGINE_API ScriptFunctionType scriptFunctionGetType(Asset* asset);

ENGINE_API void scriptFunctionSetCode(Asset* asset);
ENGINE_API const std::string& scriptFunctionGetCode(Asset* asset);

ENGINE_API float3 executeIDF(Asset* idf, float3 p);
ENGINE_API float32 executeSDF(Asset* sdf, float3 p);
ENGINE_API float32 executeODF(Asset* odf, float32 distance);
