#pragma once

#include <script_function.h>

#include "window.h"

bool8 createScriptFunctionSettingsWindow(ScriptFunction* function, Window** outWindow);
std::string scriptFunctionIdentifier(ScriptFunction* function);

