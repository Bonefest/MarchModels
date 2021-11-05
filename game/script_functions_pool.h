#pragma once

#include <string>

#include <defines.h>
#include <script_function.h>

bool8 scriptFunctionsPoolInit();
void scriptFunctionsPoolShutdown();

bool8 scriptFunctionsPoolAddPrototype(ScriptFunction* prototype);
bool8 scriptFunctionsPoolRemovePrototype(ScriptFunction* prototype);
bool8 scriptFunctionsPoolRemovePrototype(const std::string& name);

ScriptFunction* scriptFunctionsPoolFindPrototype(const std::string& name);
bool8 scriptFunctionsPoolHasPrototype(const std::string& name);
