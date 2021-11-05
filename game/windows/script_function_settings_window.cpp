#include <imgui/imgui.h>

#include <utils.h>
#include <logging.h>
#include <memory_manager.h>

#include "script_function_settings_window.h"

struct ScriptFunctionSettingsWindowData
{
  char newArgName[255];
  char codeBuf[4096];
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


  // Code frame
  ImGui::PushStyleColor(ImGuiCol_FrameBg, 0x0);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, float2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, float2(0.0f, 0.0f));      
  if(ImGui::BeginChildFrame(ImGui::GetID("Code"), float2(0.0f, 160.0f), ImGuiWindowFlags_MenuBar))
  {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, float2(0.0f, 4.0f));
    if(ImGui::BeginMenuBar())
    {
      if(ImGui::BeginMenu("File"))
      {
        ImGui::MenuItem(" " ICON_KI_ARROW_LEFT" Undo", "ctrl-z");
        ImGui::MenuItem(" " ICON_KI_ARROW_RIGHT" Redo", "ctrl-y");        
        ImGui::MenuItem(" " ICON_KI_SEARCH" Open", "ctrl-o");
        ImGui::MenuItem(" " ICON_KI_SAVE" Save", "ctrl-s");  

        ImGui::EndMenu();
      }
      
      ImGui::MenuItem(ICON_KI_WRENCH" Compile");      

      ImGui::EndMenuBar();
    }
    ImGui::PopStyleVar();

    float2 codeFrameSize = ImGui::GetWindowInnerAreaSize();

    ImGui::PushStyleColor(ImGuiCol_FrameBg, (float4)ImColor(32, 32, 32, 128));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 5.0);
      ImGui::InputTextMultiline("##CodeInputText",
                                data->codeBuf,
                                ARRAY_SIZE(data->codeBuf),
                                codeFrameSize,
                                ImGuiInputTextFlags_AllowTabInput);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
    
    ImGui::EndChildFrame();
  }
  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor();

  // Args table
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
