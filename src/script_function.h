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

#include <vector>
#include <string>
#include <unordered_map>

#include "maths/common.h"
#include "defines.h"

using std::string;
using std::vector;
using std::unordered_map;

enum ScriptFunctionType
{
  SCRIPT_FUNCTION_TYPE_SDF,
  SCRIPT_FUNCTION_TYPE_IDF,
  SCRIPT_FUNCTION_TYPE_ODF,

  SCRIPT_FUNCTION_TYPE_COUNT
};

// ----------------------------------------------------------------------------
// Script function declaration interface
// ----------------------------------------------------------------------------
ENGINE_API bool8 declareScriptFunction(ScriptFunctionType type,
                                       const string& name,
                                       const string& code,
                                       const vector<string>& parameters,
                                       bool8 redeclareIfExists = FALSE);

ENGINE_API bool8 loadScriptFunctionDeclarationFromFile(const string& filepath,
                                                       bool8 allowRedeclaration = FALSE);

ENGINE_API bool8 undeclareScriptFunction(ScriptFunctionType type, const string& name);
ENGINE_API bool8 isScriptFunctionDeclared(ScriptFunctionType type, const string& name);

// ----------------------------------------------------------------------------
// Script function interface
// ----------------------------------------------------------------------------

struct ScriptFunction;
using ScriptFunctionArgs = unordered_map<string, float32>;

ENGINE_API bool8 createScriptFunction(ScriptFunctionType type, const string& name, ScriptFunction** outFunction);
ENGINE_API void destroyScriptFunction(ScriptFunction* function);
ENGINE_API void scriptFunctionSetArgValue(ScriptFunction* function, const string& argName, float32 value);
ENGINE_API float32 scriptFunctionGetArgValue(ScriptFunction* function, const string& argName);
ENGINE_API ScriptFunctionArgs& scriptFunctionGetArgs(ScriptFunction* function);
ENGINE_API ScriptFunctionType scriptFunctionGetType(ScriptFunction* function);
ENGINE_API std::string scriptFunctionGetName(ScriptFunction* function);

ENGINE_API float3 executeIDF(ScriptFunction* idf, float3 p);
ENGINE_API float32 executeSDF(ScriptFunction* sdf, float3 p);
ENGINE_API float32 executeODF(ScriptFunction* odf, float32 distance);
