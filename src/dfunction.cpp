#include "logging.h"
#include "memory_manager.h"
#include "lua/lua_system.h"

#include "dfunction.h"

static unordered_map<string, DFunction> registeredSDFs;
static unordered_map<string, DFunction> registeredODFs;
static unordered_map<string, DFunction> registeredIDFs;

struct DFunctionInstance
{
  string name;
  DFunctionArgs arguments;
};

bool8 registerSDF(const string& name,
                  const string& code,
                  const vector<DFunctionParam>& params,
                  bool8 overwrite)
{
  sol::state& lua = luaGetMainState();
  if(registeredSDFs.find(name) != registeredSDFs.end() && overwrite == FALSE)
  {
    LOG_ERROR("Cannot register a new function: \"%s\" is already registered!", name.c_str());
    return FALSE;
  }

  lua.set(name, sol::lua_nil);
  lua.script(code);

  DFunction dfunction =
  {
    .name = name,
    .code = code,
    .params = params
  };

  registeredSDFs[name] = dfunction;
  
  return TRUE;
}

bool8 unregisterSDF(const string& name)
{
  sol::state& lua = luaGetMainState();
  if(registeredSDFs.find(name) == registeredSDFs.end())
  {
    LOG_ERROR("Cannot unregister a function: \"%s\" is not registered!", name.c_str());
    return FALSE;
  }

  lua.set(name, sol::lua_nil);
  registeredSDFs.erase(name);
  
  return TRUE;
}

bool8 createSDF(const string& name, SDF** outSDF)
{
  auto sdfIt = registeredSDFs.find(name);
  if(sdfIt == registeredSDFs.end())
  {
    LOG_ERROR("Cannot create a SDF instance!");
    return FALSE;
  }

  *outSDF = engineAllocObject<SDF>(MEMORY_TYPE_GENERAL);
  SDF* sdf = *outSDF;
  sdf->name = name;

  for(const DFunctionParam& param: sdfIt->second.params)
  {
    sdf->arguments[param.name] = param.defaultValue;
  }
  
  return TRUE;
}

DFunctionArgs& sdfGetArguments(SDF* sdf)
{
  return sdf->arguments;
}

float32 executeSDF(SDF* sdf, float3 p)
{
  sol::state& lua = luaGetMainState();

  // These values are filled before image rendering is started
  // lua["args"]["time"] = 0.0f;
  // lua["args"]["material"] = ...;
  
  for(auto argument: sdf->arguments)
  {
    lua["args"][argument.first] = argument.second;
  }

  lua["args"]["p"] = convertFloat3ToLua(p);
  
  float32 distance = lua[sdf->name]();

  return distance;
}
