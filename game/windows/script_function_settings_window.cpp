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
  ScriptFunctionSettingsWindowData* data = (ScriptFunctionSettingsWindowData*)windowGetInternalData(window);

  if(ImGui::BeginTable("ScriptFunctionArgsTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
  {
    ScriptFunctionArgs& args = scriptFunctionGetArgs(data->function);

    ImGui::TableSetupColumn("##Commands");
    ImGui::TableSetupColumn("ID");
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Value");

    ImGui::TableHeadersRow();
    
    for(uint32 argIdx = 0; argIdx < args.size(); argIdx++)
    {
      ImGui::TableNextRow();


      
      ImGui::TableSetColumnIndex(0);
      // ImGui::SetColumnWidth(-1, 32);
      ImGui::Button("X");


      
      // ID column
      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%d", argIdx);

      // Name column
      ImGui::TableSetColumnIndex(2);
      
      // Value column
      ImGui::TableSetColumnIndex(3);

    }

    ImGui::EndTable();
  }
  
  ImGui::Button("Test");
}

void scriptFunctionSettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{

}
