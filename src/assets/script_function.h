/**
 * Script function is a function that can be registered from a C++ side in Lua state.
 * Each such function has a name, code and list of parameters.
 *
 * Script function can be saved and loaded from the file, since it's basically a description.
 *
 * @note: Parameter can have only float32 type.
 *
 * @note: The next parameters are pre-defined:
 *   time - Equals the time elapsed since the application has started
 *   camera - A lua-table, which contains all information about a main view camera
 *   material - In case if SDF is evaluated, contains information about a material of a shape
 */

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

ENGINE_API const char* scriptFunctionTypeLabel(ScriptFunctionType type);

ENGINE_API bool8 createScriptFunction(ScriptFunctionType type,
                                      const std::string& name,
                                      Asset** outAsset);

ENGINE_API void scriptFunctionCopy(Asset* dst, Asset* src);
ENGINE_API Asset* scriptFunctionClone(Asset* assetCloneFrom);

ENGINE_API void scriptFunctionSetArgValue(Asset* asset, const std::string& argName, float32 value);
ENGINE_API float32 scriptFunctionGetArgValue(Asset* asset, const std::string& argName);
ENGINE_API ScriptFunctionArgs& scriptFunctionGetArgs(Asset* asset);

ENGINE_API void scriptFunctionSetType(Asset* asset, ScriptFunctionType type);
ENGINE_API ScriptFunctionType scriptFunctionGetType(Asset* asset);

ENGINE_API void scriptFunctionSetCode(Asset* asset, const std::string& code);

/**
 * @return code string with working symbols (parameter names, built-in parameters)
 * represents a string how it looks from user point of view.
 */
ENGINE_API const std::string& scriptFunctionGetRawCode(Asset* asset);

/**
 * @return code string prepapred for integration in GLSL code (parameters are replaced
 * by float constats
 *
 * @warning It's a costly function. It's better to save the result somewhere if you
 * need to reuse it often.
 */
ENGINE_API std::string scriptFunctionGetGLSLCode(Asset* asset);

ENGINE_API float3 executeIDF(Asset* idf, float3 p);
ENGINE_API float32 executeSDF(Asset* sdf, float3 p);
ENGINE_API float32 executeODF(Asset* odf, float32 distance);
