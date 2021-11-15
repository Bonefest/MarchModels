#pragma once

#include <assets/geometry.h>
#include <assets/script_function.h>

#include "window.h"

bool8 createScriptFunctionSettingsWindow(AssetPtr owner, AssetPtr function, Window** outWindow);

std::string scriptFunctionWindowIdentifier(Asset* function);

