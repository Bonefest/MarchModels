#pragma once

#include <sol/sol.hpp>

#include "defines.h"

bool8 initializeLuaSystem();
void shutdownLuaSystem();

ENGINE_API sol::state& luaGetMainState();
