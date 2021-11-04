#include <imgui/imgui.h>

#include <utils.h>
#include <logging.h>
#include <memory_manager.h>

#include "script_function_settings_window.h"

struct ScriptFunctionSettingsWindowData
{
  char newArgName[255];
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

  if(allocateWindow(interface, scriptFunctionWindowIdentifier(function), outWindow) == FALSE)
  {
    return FALSE;
  }

  ScriptFunctionSettingsWindowData* data = engineAllocObject<ScriptFunctionSettingsWindowData>(MEMORY_TYPE_GENERAL);
  data->function = function;
  windowSetInternalData(*outWindow, data);

  return TRUE;
}

std::string scriptFunctionWindowIdentifier(ScriptFunction* function)
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
  ScriptFunctionArgs& args = scriptFunctionGetArgs(data->function);
  
  ImGuiStyle& style = ImGui::GetStyle();
  
  if(ImGui::BeginTable("ScriptFunctionArgsTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp))
  {
    ImGui::TableSetupColumn("##Commands", 0, 0.05f);
    ImGui::TableSetupColumn("ID", 0, 0.05f);
    ImGui::TableSetupColumn("Name", 0, 0.45f);
    ImGui::TableSetupColumn("Value", 0, 0.45f);
    ImGui::TableHeadersRow();

    ImGui::PushStyleColor(ImGuiCol_FrameBg, 0x0);
    
    int32 argIdx = 0;
    for(auto argIt = args.begin(); argIt != args.end();)
    {
      bool8 removeRequested = FALSE;
      
      ImGui::PushID(argIt->first.c_str());
      
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        
        float32 cellWidth = ImGui::GetContentRegionAvail().x;

        ImGui::PushStyleColor(ImGuiCol_Button, (float4)ImColor(128, 0, 0, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (float4)ImColor(160, 0, 0, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (float4)ImColor(144, 0, 0, 255));                
          if(ImGui::Button("Del", float2(cellWidth, 0.0f)))
          {
            removeRequested = TRUE;
          }
        ImGui::PopStyleColor(3);

          
        // ID column
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", argIdx);

        // Name column
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%s", argIt->first.c_str());

        // Value column
        ImGui::TableSetColumnIndex(3);
        ImGui::InputFloat("##ArgValueInput", &argIt->second, 0.0f, 0.0f, "%.1f");

      ImGui::PopID();

      if(removeRequested == FALSE)
      {
        argIt++;
        argIdx++;
      }
      else
      {
        argIt = args.erase(argIt);
      }
    }

    ImGui::PopStyleColor();
    ImGui::EndTable();
  }

  ImGui::InputTextWithHint("##NewArgNameInput", "New argument name", data->newArgName, ARRAY_SIZE(data->newArgName));
  ImGui::SameLine();

  bool8 argNameAlreadyExists = args.find(data->newArgName) != args.end();

  ImGui::BeginDisabled(argNameAlreadyExists ? TRUE : FALSE);
  if(ImGui::Button("Create") && argNameAlreadyExists == FALSE)
  {
    args[data->newArgName] = 0.0f;
    data->newArgName[0] = '\0';
  }
  ImGui::EndDisabled();
  
}

void scriptFunctionSettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{

}
