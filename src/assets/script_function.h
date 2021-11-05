#pragma once

#include "asset.h"

static const AssetType ASSET_TYPE_SCRIPT_FUNCTION = 0xdf426230;

enum ScriptFunctionType
{
  SCRIPT_FUNCTION_TYPE_SDF,
  SCRIPT_FUNCTION_TYPE_IDF,
  SCRIPT_FUNCTION_TYPE_ODF,

  SCRIPT_FUNCTION_TYPE_COUNT
};


ENGINE_API bool8 createScriptFunction(ScriptFunctionType type,
                                      const string& name,
                                      Asset** outFunction);

ENGINE_API void scriptFunctionSetArgValue(ScriptFunction* function, const string& argName, float32 value);
ENGINE_API float32 scriptFunctionGetArgValue(ScriptFunction* function, const string& argName);
ENGINE_API ScriptFunctionArgs& scriptFunctionGetArgs(ScriptFunction* function);
ENGINE_API ScriptFunctionType scriptFunctionGetType(ScriptFunction* function);

ENGINE_API float3 executeIDF(ScriptFunction* idf, float3 p);
ENGINE_API float32 executeSDF(ScriptFunction* sdf, float3 p);
ENGINE_API float32 executeODF(ScriptFunction* odf, float32 distance);
