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

  return TRUE;
}

void shutdownLuaSystem()
{

}

sol::state& luaGetMainState()
{
  return mainLuaState;
}
