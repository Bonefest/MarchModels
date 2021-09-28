/**
 * DFunction is named after common letter in SDF, ODF, IDF abbreviations.
 *
 * DFunction is a function that can be registered from a C++ side in Lua state.
 * Each such function has a name, code and list of parameters.
 *
 * DFunction can be saved and loaded from the file, since it's basically a description.
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

struct DFunctionParam
{
  string name;
  float32 defaultValue;
};

using DFunctionParams = vector<DFunctionParam>;

struct DFunction
{
  string name;
  string code;
  DFunctionParams params;
};

using DFunctionArgs = unordered_map<string, float32>;

struct DFunctionInstance;

using SDF = DFunctionInstance;
using ODF = DFunctionInstance;
using IDF = DFunctionInstance;

ENGINE_API bool8 registerSDF(const string& name,
                             const string& code,
                             const vector<DFunctionParam>& params,
                             bool8 overwrite = FALSE);
ENGINE_API bool8 regiterSDFFromFile(const string& filename, bool8 overwrite = FALSE);
ENGINE_API bool8 unregisterSDF(const string& name);
ENGINE_API bool8 createSDF(const string& name, SDF** outSDF);
ENGINE_API bool8 isSDFRegistered(const string& name);
ENGINE_API DFunctionArgs& sdfGetArguments(SDF* sdf);
ENGINE_API float32 executeSDF(SDF* sdf, float3 p);
