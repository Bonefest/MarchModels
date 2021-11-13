#include <imgui/imgui.h>

#include <utils.h>
#include <logging.h>
#include <memory_manager.h>
#include <assets/assets_manager.h>
#include <assets/script_function.h>

#include "ui_utils.h"
#include "script_function_settings_window.h"

static const char* NewSaveNamePopupName = "NewSaveName##ScriptFunctionSettingsWindow";

struct ScriptFunctionSettingsWindowData
{
  char saveName[255];
  char newArgName[255];
  char codeBuf[4096];
  Asset* function;
};

static char tempSaveName[255];

static bool8 scriptFunctionSettingsWindowInitialize(Window*);
static void scriptFunctionSettingsWindowShutdown(Window*);
static void scriptFunctionSettingsWindowUpdate(Window* window, float64 delta);
static void scriptFunctionSettingsWindowDraw(Window* window, float64 delta);
static void scriptFunctionSettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender);

static void saveFunction(ScriptFunctionSettingsWindowData* windowData);

bool8 createScriptFunctionSettingsWindow(Asset* function, Window** outWindow)
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
  strcpy(data->saveName, assetGetName(function).c_str());
  windowSetInternalData(*outWindow, data);

  return TRUE;
}

std::string scriptFunctionWindowIdentifier(Asset* function)
{
  char identifier[128];
  sprintf(identifier, "%s settings##%p", assetGetName(function).c_str(), function);
  return identifier;
}

bool8 scriptFunctionSettingsWindowInitialize(Window* window)
{
  return TRUE;
}

void scriptFunctionSettingsWindowShutdown(Window* window)
{
  ScriptFunctionSettingsWindowData* data = (ScriptFunctionSettingsWindowData*)windowGetInternalData(window);
  if(data != nullptr)
  {
    engineFreeObject(data, MEMORY_TYPE_GENERAL);
  }
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
  bool8 needOpenSavePopup = FALSE;
  
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
        if(ImGui::MenuItem(" " ICON_KI_SAVE" Save", "ctrl-s"))
        {
          saveFunction(data);
        }
        if(ImGui::MenuItem(" " ICON_KI_SAVE" Save as", "ctrl-shift-s"))
        {
          needOpenSavePopup = TRUE;
          strcpy(textInputPopupGetBuffer(), data->saveName);
        }

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

  if(needOpenSavePopup == TRUE)
  {
    ImGui::OpenPopup(NewSaveNamePopupName);
  }
  
  // Save name popup
  if(ImGui::IsPopupOpen(NewSaveNamePopupName))
  {
    ImGuiUtilsButtonsFlags pressedButton = textInputPopup(NewSaveNamePopupName, "Enter save name");
                                                          
    if(ImGuiUtilsButtonsFlags_Accept == pressedButton)
    {
      const char* enteredName = textInputPopupGetBuffer();
      
      if(strlen(enteredName) == 0)
      {
        // TODO: Show error popup
      }
      else
      {
        strcpy(data->saveName, enteredName);
        saveFunction(data);
      }
    }
  }
  
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

void saveFunction(ScriptFunctionSettingsWindowData* windowData)
{
  Asset* assetToSave = windowData->function;
  Asset* prototypeAsset = assetsManagerFindAsset(windowData->saveName);

  // If name already taken and save name is not equal to the script function's name - show warning
  if(prototypeAsset != nullptr && strcmp(windowData->saveName, assetGetName(assetToSave).c_str()) != 0)
  {
    // TODO: show warning popup
  }

  if(prototypeAsset != nullptr)
  {
    scriptFunctionCopy(prototypeAsset, assetToSave);
  }
  else
  {
    prototypeAsset = scriptFunctionClone(assetToSave);
    assetSetName(prototypeAsset, windowData->saveName);
    assetsManagerAddAsset(prototypeAsset);
  }

  LOG_INFO("Script function '%s' was saved with name '%s'",
           assetGetName(assetToSave).c_str(),
           windowData->saveName);
}
