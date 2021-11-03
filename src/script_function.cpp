#include <limits>

#include "logging.h"
#include "memory_manager.h"
#include "lua/lua_system.h"
#include "script_function.h"

struct ScriptFunctionDeclaration
{
  string name;
  string code;
  vector<string> parameters;
};

struct ScriptFunction
{
  string name;
  ScriptFunctionType type;
  ScriptFunctionArgs args;
};

using ScriptFunctionsMap = unordered_map<string, ScriptFunctionDeclaration>;
static ScriptFunctionsMap functionMaps[SCRIPT_FUNCTION_TYPE_COUNT];

// ----------------------------------------------------------------------------
// Script function declaration interface
// ----------------------------------------------------------------------------

bool8 declareScriptFunction(ScriptFunctionType type,
                            const string& name,
                            const string& code,
                            const vector<string>& parameters,
                            bool8 redeclareIfExists)
{
  ScriptFunctionsMap& map = functionMaps[type];
  if(isScriptFunctionDeclared(type, name) && redeclareIfExists == FALSE)
  {
    LOG_ERROR("Cannot declare a script function: \"%s\" is already declared!", name);
    return FALSE;
  }

  char fullCodeBuf[4096];
  sprintf(fullCodeBuf,
          "function %s()\n"
          "  %s         \n"
          "end          \n",
          name.c_str(), code.c_str());

  sol::state& lua = luaGetMainState();
  lua.set(name, sol::lua_nil);
  lua.script(fullCodeBuf);

  map[name] = ScriptFunctionDeclaration { name, code, parameters };
  
  return TRUE;
}

bool8 undeclareScriptFunction(ScriptFunctionType type, const string& name)
{
  ScriptFunctionsMap& map = functionMaps[type];
  if(!isScriptFunctionDeclared(type, name))
  {
    LOG_ERROR("Cannot undeclare a function: \"%s\" is not declared!", name.c_str());
    return FALSE;
  }

  sol::state& lua = luaGetMainState();
  
  lua.set(name, sol::lua_nil);
  map.erase(name);
  
  return TRUE;  
}

bool8 isScriptFunctionDeclared(ScriptFunctionType type, const string& name)
{
  ScriptFunctionsMap& map = functionMaps[type];
  return map.find(name) != map.end();
}

// ----------------------------------------------------------------------------
// Script function interface
// ----------------------------------------------------------------------------

bool8 createScriptFunction(ScriptFunctionType type, const string& name, ScriptFunction** outFunction)
{
  ScriptFunctionsMap& map = functionMaps[type];
  auto scriptFunctionDeclarationIt = map.find(name);
  assert(scriptFunctionDeclarationIt != map.end());

  ScriptFunctionDeclaration& declaration = scriptFunctionDeclarationIt->second;
  
  *outFunction = engineAllocObject<ScriptFunction>(MEMORY_TYPE_GENERAL);
  ScriptFunction* function = *outFunction;
  function->name = name;
  function->type = type;
  
  for(const string& parameter: declaration.parameters)
  {
    function->args[parameter] = 0.0f;
  }

  return TRUE;
}

void destroyScriptFunction(ScriptFunction* function)
{
  engineFreeObject<ScriptFunction>(function, MEMORY_TYPE_GENERAL);
}

void scriptFunctionSetArgValue(ScriptFunction* function, const string& argName, float32 value)
{
  // TODO: Check if argument is declared
  function->args[argName] = value;
}

float32 scriptFunctionGetArgValue(ScriptFunction* function, const string& argName)
{
  return function->args[argName];
}

ScriptFunctionArgs& scriptFunctionGetArgs(ScriptFunction* function)
{
  return function->args;
}

ScriptFunctionType scriptFunctionGetType(ScriptFunction* function)
{
  return function->type;
}

std::string scriptFunctionGetName(ScriptFunction* function)
{
  return function->name;
}

float3 executeIDF(ScriptFunction* idf, float3 p)
{
  if(idf == nullptr)
  {
    return p;
  }

  assert(idf->type == SCRIPT_FUNCTION_TYPE_IDF);
  
  sol::state& lua = luaGetMainState();
  
  for(const auto& argument: idf->args)
  {
    lua["args"][argument.first] = argument.second;
  }

  lua["args"]["p"] = convertFloat3ToLua(p);
  
  return p; //lua[idf->name]();
}

float32 executeSDF(ScriptFunction* sdf, float3 p)
{
  if(sdf == nullptr)
  {
    return std::numeric_limits<float32>::max();
  }
  
  assert(sdf->type == SCRIPT_FUNCTION_TYPE_SDF);
  
  sol::state& lua = luaGetMainState();
  
  for(const auto& argument: sdf->args)
  {
    lua["args"][argument.first] = argument.second;
  }

  lua["args"]["p"] = convertFloat3ToLua(p);
  
  return lua[sdf->name]();
}

float32 executeODF(ScriptFunction* odf, float32 distance)
{
  if(odf == nullptr)
  {
    return distance;
  }
  
  assert(odf->type == SCRIPT_FUNCTION_TYPE_ODF);
  
  sol::state& lua = luaGetMainState();
  
  for(const auto& argument: odf->args)
  {
    lua["args"][argument.first] = argument.second;
  }

  lua["args"]["distance"] = distance;
  
  return distance; //lua[odf->name]();
}
