
#include "memory_manager.h"
#include "lua/lua_system.h"
#include "script_function.h"

using std::string;
using std::vector;

struct ScriptFunction
{
  string code;
  ScriptFunctionArgs args;
  ScriptFunctionType type;
};

static void scriptFunctionDestroy(Asset* asset);
static bool8 scriptFunctionSerialize(Asset* asset) { /** TODO */ }
static bool8 scriptFunctionDeserialize(Asset* asset) { /** TODO */ }
static uint32 scriptFunctionGetSize(Asset* asset) { /** TODO */ }

bool8 createScriptFunction(ScriptFunctionType type, const string& name, Asset** outAsset)
{
  AssetInterface interface = {};
  interface.destroy = scriptFunctionDestroy;
  interface.serialize = scriptFunctionSerialize;
  interface.deserialize = scriptFunctionDeserialize;
  interface.getSize = scriptFunctionGetSize;
  interface.type = ASSET_TYPE_SCRIPT_FUNCTION;

  assert(allocateAsset(interface, name, outAsset));
  
  ScriptFunction* function = engineAllocObject<ScriptFunction>(MEMORY_TYPE_GENERAL);
  function->code = "";
  function->type = type;

  assetSetInternalData(*outAsset, function);
  
  return TRUE;
}

Asset* scriptFunctionClone(Asset* assetCloneFrom)
{
  Asset* copy;
  assert(createScriptFunction((ScriptFunctionType)0, "", &copy));
  scriptFunctionCopy(copy, assetCloneFrom);

  return copy;
}

void scriptFunctionCopy(Asset* dst, Asset* src)
{
  ScriptFunction* srcData = (ScriptFunction*)assetGetInternalData(src);  
  ScriptFunction* dstData = (ScriptFunction*)assetGetInternalData(dst);

  *dstData = *srcData;
  assetSetName(dst, assetGetName(src));
}

void scriptFunctionDestroy(Asset* asset)
{
  ScriptFunction* data = (ScriptFunction*)assetGetInternalData(asset);
  
  engineFreeObject<ScriptFunction>(data, MEMORY_TYPE_GENERAL);
}

void scriptFunctionSetArgValue(Asset* asset, const string& argName, float32 value)
{
  ScriptFunction* data = (ScriptFunction*)assetGetInternalData(asset);
  
  data->args[argName] = value;
}

float32 scriptFunctionGetArgValue(Asset* asset, const string& argName)
{
  ScriptFunction* data = (ScriptFunction*)assetGetInternalData(asset);
  
  return data->args[argName];
}

ScriptFunctionArgs& scriptFunctionGetArgs(Asset* asset)
{
  ScriptFunction* data = (ScriptFunction*)assetGetInternalData(asset);
  
  return data->args;
}

ScriptFunctionType scriptFunctionGetType(Asset* asset)
{
  ScriptFunction* data = (ScriptFunction*)assetGetInternalData(asset);
  
  return data->type;
}

void scriptFunctionSetCode(Asset* asset, const std::string& code)
{
  ScriptFunction* data = (ScriptFunction*)assetGetInternalData(asset);

  data->code = code;

  char fullCodeBuf[4096];
  sprintf(fullCodeBuf,
          "function %s()\n"
          "  %s         \n"
          "end          \n",
          assetGetName(asset).c_str(), code.c_str());

  sol::state& lua = luaGetMainState();
  lua.set(assetGetName(asset), sol::lua_nil);
  lua.script(fullCodeBuf);
}

const std::string& scriptFunctionGetCode(Asset* asset)
{
  ScriptFunction* data = (ScriptFunction*)assetGetInternalData(asset);
  
  return data->code;
}

float3 executeIDF(Asset* idf, float3 p)
{
  if(idf == nullptr)
  {
    return p;
  }

  ScriptFunction* data = (ScriptFunction*)assetGetInternalData(idf);
  
  assert(data->type == SCRIPT_FUNCTION_TYPE_IDF);
  
  sol::state& lua = luaGetMainState();
  
  for(const auto& argument: data->args)
  {
    lua["args"][argument.first] = argument.second;
  }

  lua["args"]["p"] = convertFloat3ToLua(p);
  
  return p; //lua[idf->name]();
}

float32 executeSDF(Asset* sdf, float3 p)
{
  if(sdf == nullptr)
  {
    return std::numeric_limits<float32>::max();
  }

  ScriptFunction* data = (ScriptFunction*)assetGetInternalData(sdf);  
  
  assert(data->type == SCRIPT_FUNCTION_TYPE_SDF);
  
  sol::state& lua = luaGetMainState();
  
  for(const auto& argument: data->args)
  {
    lua["args"][argument.first] = argument.second;
  }

  lua["args"]["p"] = convertFloat3ToLua(p);
  
  return lua[assetGetName(sdf)]();
}

float32 executeODF(Asset* odf, float32 distance)
{
  if(odf == nullptr)
  {
    return distance;
  }

  ScriptFunction* data = (ScriptFunction*)assetGetInternalData(odf);
  
  assert(data->type == SCRIPT_FUNCTION_TYPE_ODF);
  
  sol::state& lua = luaGetMainState();
  
  for(const auto& argument: data->args)
  {
    lua["args"][argument.first] = argument.second;
  }

  lua["args"]["distance"] = distance;
  
  return distance; //lua[odf->name]();
}
