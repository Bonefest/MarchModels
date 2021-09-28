#pragma once

#include <sol/sol.hpp>

#include "defines.h"
#include "maths/common.h"

bool8 initializeLuaSystem();
void shutdownLuaSystem();

ENGINE_API sol::state& luaGetMainState();
ENGINE_API float3 convertFloat3FromLua(const sol::table& luaFloat3);
ENGINE_API sol::table convertFloat3ToLua(float3 vector);
