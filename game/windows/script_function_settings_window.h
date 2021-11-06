#pragma once

#include <assets/script_function.h>

#include "window.h"

bool8 createScriptFunctionSettingsWindow(Asset* function, Window** outWindow);
std::string scriptFunctionWindowIdentifier(Asset* function);

