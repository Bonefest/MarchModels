#pragma once

#include <geometry.h>
#include <assets/script_function.h>

#include "window.h"

bool8 createScriptFunctionSettingsWindow(Geometry* owner, AssetPtr function, Window** outWindow);

std::string scriptFunctionWindowIdentifier(Asset* function);

