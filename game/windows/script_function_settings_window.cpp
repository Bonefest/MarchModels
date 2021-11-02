#include <imgui/imgui.h>

#include <memory_manager.h>

#include "script_function_settings_window.h"

struct ScriptFunctionSettingsWindowData
{
  ScriptFunction* function;
};

static bool8 scriptFunctionSettingsWindowInitialize(Window*);
static void scriptFunctionSettingsWindowShutdown(Window*);
static void scriptFunctionSettingsWindowUpdate(Window* window, float64 delta);
static void scriptFunctionSettingsWindowDraw(Window* window, float64 delta);
static void scriptFunctionSettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender);

bool8 createScriptFunctionSettingsWindow(ScriptFunction* function, Window** outWindow)
{
  WindowInterface interface = {};
  interface.initialize = scriptFunctionSettingsWindowInitialize;
  interface.shutdown = scriptFunctionSettingsWindowShutdown;
  interface.update = scriptFunctionSettingsWindowUpdate;
  interface.draw = scriptFunctionSettingsWindowDraw;
  interface.processInput = scriptFunctionSettingsWindowProcessInput;

  if(allocateWindow(interface, scriptFunctionIdentifier(function), outWindow) == FALSE)
  {
    return FALSE;
  }

  ScriptFunctionSettingsWindowData* data = engineAllocObject<ScriptFunctionSettingsWindowData>(MEMORY_TYPE_GENERAL);
  data->function = function;
  windowSetInternalData(*outWindow, data);

  return TRUE;
}

std::string scriptFunctionIdentifier(ScriptFunction* function)
{
  char identifier[128];
  sprintf(identifier, "%s settings##%p", scriptFunctionGetName(function).c_str(), function);
  return identifier;
}

bool8 scriptFunctionSettingsWindowInitialize(Window* window)
{
  return TRUE;
}

void scriptFunctionSettingsWindowShutdown(Window* window)
{

}

void scriptFunctionSettingsWindowUpdate(Window* window, float64 delta)
{

}

void scriptFunctionSettingsWindowDraw(Window* window, float64 delta)
{
  ImGui::Button("Test");
}

void scriptFunctionSettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{

}
