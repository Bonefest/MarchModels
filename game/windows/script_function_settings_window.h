#pragma once

#include <assets/script_function.h>

#include "window.h"

bool8 createScriptFunctionSettingsWindow(AssetPtr function, Window** outWindow);

std::string scriptFunctionWindowIdentifier(Asset* function);

