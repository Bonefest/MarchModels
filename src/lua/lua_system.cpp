#include "lua_system.h"

static sol::state mainLuaState;

bool8 initializeLuaSystem()
{
  //sol::table searchers = mainLuaState["package"]["searchers"];

  mainLuaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);
  auto result = mainLuaState.script_file("scripts/internal_system_init.lua");
  if(!result.valid())
  {
    return FALSE;
  }

  mainLuaState.create_table("args");
  
  return TRUE;
}

void shutdownLuaSystem()
{

}

sol::state& luaGetMainState()
{
  return mainLuaState;
}

float3 convertFloat3FromLua(const sol::table& luaFloat3)
{
  return float3(luaFloat3["x"], luaFloat3["y"], luaFloat3["z"]);
}

sol::table convertFloat3ToLua(float3 vector)
{
  char scriptBuf[64];
  sprintf(scriptBuf, "float3(%f, %f, %f)", vector.x, vector.y, vector.z);
  
  return mainLuaState["float3"](vector.x, vector.y, vector.z);
}
