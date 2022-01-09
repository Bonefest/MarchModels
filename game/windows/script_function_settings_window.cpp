#include <deque>
#include <string>

using std::deque;
using std::string;

#include <imgui/imgui.h>

#include <utils.h>
#include <logging.h>
#include <memory_manager.h>
#include <assets/assets_manager.h>
#include <assets/script_function.h>

#include "editor.h"
#include "ui_utils.h"
#include "ui_styles.h"
#include "script_function_settings_window.h"

static const char* NewSaveNamePopupName = "NewSaveName##ScriptFunctionSettingsWindow";

struct ScriptFunctionSettingsWindowData
{
  char saveName[255];
  char newArgName[255];
  char codeBuf[4096];
  std::deque<string> logMessages;
  
  AssetPtr owner;
  AssetPtr function;

  AssetPtr dummyGeometry;
};

static bool8 scriptFunctionSettingsWindowInitialize(Window*);
static void scriptFunctionSettingsWindowShutdown(Window*);
static void scriptFunctionSettingsWindowUpdate(Window* window, float64 delta);
static void scriptFunctionSettingsWindowDraw(Window* window, float64 delta);
static void scriptFunctionSettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender);

static void saveFunction(ScriptFunctionSettingsWindowData* windowData);

static bool8 notifyGeometryScriptFunctionHasChanged(Asset* geometry, void* changedScriptFunction)
{
  if(geometryHasFunction(geometry, (Asset*)changedScriptFunction) == TRUE)
  {
    geometryNotifyFunctionHasChanged(geometry, (Asset*)changedScriptFunction);
  }

  return FALSE;
}

bool8 createScriptFunctionSettingsWindow(AssetPtr owner, AssetPtr function, Window** outWindow)
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
  data->owner = owner;
  data->function = function;
  strcpy(data->saveName, assetGetName(function).c_str());
  strcpy(data->codeBuf, scriptFunctionGetRawCode(function).c_str());
  windowSetInternalData(*outWindow, data);

  Asset* dummyGeometry;
  createGeometry("dummy", &dummyGeometry);
  data->dummyGeometry = AssetPtr(dummyGeometry);
  
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
  ImGuiStyle& imstyle = ImGui::GetStyle();  

  ScriptFunctionSettingsWindowData* data = (ScriptFunctionSettingsWindowData*)windowGetInternalData(window);
  ScriptFunctionType sfType = scriptFunctionGetType(data->function);  
  AssetPtr assetFromManager = assetsManagerFindAsset(assetGetName(data->function));
  bool8 isPrototype = assetFromManager != nullptr &&
    (assetGetInternalData(assetFromManager) == assetGetInternalData(data->function));
  
  pushIconSmallButtonStyle();
  
  // --------------------------------------------------------------------------
  // Open button
  // --------------------------------------------------------------------------
  ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);  
  ImGui::SmallButton("[Open]");
  ImGui::SameLine();
  ImGui::PopStyleColor();

  // --------------------------------------------------------------------------
  // Save button
  // --------------------------------------------------------------------------
  ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);    
  if(ImGui::SmallButton("[Save]"))
  {
    saveFunction(data);
  }
  ImGui::SameLine();
  ImGui::PopStyleColor();  

  // --------------------------------------------------------------------------
  // Save as button
  // --------------------------------------------------------------------------
  ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);    
  if(ImGui::SmallButton("[Save as]"))
  {
    strcpy(textInputPopupGetBuffer(), data->saveName);
    ImGui::OpenPopup(NewSaveNamePopupName);    
  }
  ImGui::SameLine();
  ImGui::PopStyleColor();  

  // --------------------------------------------------------------------------
  // Forking buttons
  // --------------------------------------------------------------------------
  ImGui::PushStyleColor(ImGuiCol_Text, (float4)WarningClr);
  if(isPrototype == TRUE)
  {
    if(ImGui::SmallButton("[" ICON_KI_USERS " Prototype]"))
    {
        geometryRemoveFunction(data->owner, data->function);
        data->function = AssetPtr(scriptFunctionClone(assetFromManager));
        geometryAddFunction(data->owner, data->function);
    }
  }
  else
  {
    bool8 prototypeExists = assetFromManager != nullptr;

    ImGui::BeginDisabled(prototypeExists == FALSE);    

    // NOTE: We can unfork back only if prototype exists
    if(ImGui::SmallButton("[" ICON_KI_USER " Instance]") && prototypeExists == TRUE)
    {
      // NOTE: Replace script function by a function from the assets manager
      geometryRemoveFunction(data->owner, data->function);
      data->function = assetFromManager;
      geometryAddFunction(data->owner, data->function);
    }

    // if(ImGui::SmallButton("[Sync]") ...
    ImGui::EndDisabled();    
  }

  ImGui::PopStyleColor();
  ImGui::SameLine();
  
  // --------------------------------------------------------------------------
  // Function type changing button
  // --------------------------------------------------------------------------
  ImGui::PushStyleColor(ImGuiCol_Text, (float4)WarningClr);  
  
  char typeButton[32];
  sprintf(typeButton, "[%s]", scriptFunctionTypeLabel(sfType));
  if(ImGui::SmallButton(typeButton) && isPrototype == TRUE)
  {
    sfType = (ScriptFunctionType)(((int)sfType + 1) % (int)SCRIPT_FUNCTION_TYPE_COUNT);
    scriptFunctionSetType(data->function, sfType);
  }
  
  ImGui::PopStyleColor();  
  popIconSmallButtonStyle();




  
  float2 avalReg = ImGui::GetContentRegionAvail();
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, float2(0.0, 0.0));
  // --------------------------------------------------------------------------
  // Code child
  // --------------------------------------------------------------------------
  ImGui::BeginChild("CodeChild", float2(avalReg.x * 0.5, avalReg.y));
  
  float2 cursorPos = ImGui::GetCursorScreenPos();
  ImGui::GetWindowDrawList()->AddRectFilled(cursorPos,
                                            float2(cursorPos.x + avalReg.x * 0.5, cursorPos.y + ImGui::GetFontSize()),
                                            ImColor(imstyle.Colors[ImGuiCol_MenuBarBg]),
                                            2.0);  
  
  ImGui::Text("Code");
  ImGui::SameLine(avalReg.x * 0.5 - 80);

  pushIconSmallButtonStyle();
  if(ImGui::SmallButton(ICON_KI_WRENCH " Compile"))
  {
    geometryRemoveFunction(data->dummyGeometry, data->function);
    geometryAddFunction(data->dummyGeometry, data->function);

    std::string previousCode = scriptFunctionGetRawCode(data->function);
    scriptFunctionSetCode(data->function, data->codeBuf);

    bool8 compilationSucceeded = geometryGetProgram(data->dummyGeometry) != nullptr ? TRUE : FALSE;
    if(compilationSucceeded == TRUE)
    {
      geometryTraversePostorder(sceneGetGeometryRoot(editorGetCurrentScene()),
                                notifyGeometryScriptFunctionHasChanged,
                                data->function);

    }
    else
    {
      scriptFunctionSetCode(data->function, previousCode);
    }

    char logMessage[255];
    sprintf(logMessage,
            "_<C>%#010x</C>_ Compilation has %s.",
            revbytes((uint32)(compilationSucceeded == TRUE ? LogInfoClr : LogErrorClr)),
            (compilationSucceeded == TRUE ? "succeeded" : "failed"));

    data->logMessages.push_front(logMessage);
    
  }
  popIconSmallButtonStyle();
  
  float2 codeReg = ImGui::GetContentRegionAvail();

  ImGui::PushStyleColor(ImGuiCol_FrameBg, (float4)ImColor(32, 32, 32, 128));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 5.0);  
    ImGui::InputTextMultiline("##CodeInputText",
                              data->codeBuf,
                             ARRAY_SIZE(data->codeBuf),
                              codeReg,
                              ImGuiInputTextFlags_AllowTabInput);
  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor();
  
  ImGui::EndChild();

  // --------------------------------------------------------------------------
  ImGui::SameLine();
  ImGui::BeginChild("RightChildren", float2(avalReg.x * 0.5, avalReg.y));    
  {
    // ------------------------------------------------------------------------
    // Arguments child
    // ------------------------------------------------------------------------

    ImGui::BeginChild("ArgumentsChild", float2(avalReg.x * 0.5, avalReg.y * 0.5));
    if(ImGui::BeginTabBar("##tabs"))
    {
      if(ImGui::BeginTabItem("Custom arguments"))
      {

        // --- Arguments table
        ScriptFunctionArgs& args = scriptFunctionGetArgs(data->function);

        if(ImGui::BeginTable("ScriptFunctionArgsTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp))
        {
          ImGui::TableSetupColumn("##Commands", 0, 0.07f);
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
                if(ImGui::Button("X", float2(cellWidth, 0.0f)))
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
        }

        ImGui::PopStyleColor();
        ImGui::EndTable();        
        
        // --- New argument creation
        ImGui::Dummy(float2(0.0, 10.0));
        ImGui::InputTextWithHint("##NewArgNameInput", "New argument name", data->newArgName, ARRAY_SIZE(data->newArgName));
        ImGui::SameLine();
        ImGui::Dummy(float2(10.0, 0.0));
        ImGui::SameLine();        
        
        bool8 argNameAlreadyExists = args.find(data->newArgName) != args.end();


        ImGui::BeginDisabled(argNameAlreadyExists ? TRUE : FALSE);
        if(ImGui::Button("Create") && argNameAlreadyExists == FALSE)
        {
          args[data->newArgName] = 0.0f;
          data->newArgName[0] = '\0';
        }
        ImGui::EndDisabled();
        

        ImGui::EndTabItem();
      }
      if(ImGui::BeginTabItem("Built-in arguments"))
      {
        ImGui::Text("T");

        ImGui::EndTabItem();        
      }

      ImGui::EndTabBar();

    }
    
    ImGui::EndChild();

    // ------------------------------------------------------------------------
    // Compile log child
    // ------------------------------------------------------------------------
    ImGui::BeginChild("CompileLogChild", float2(avalReg.x * 0.5, avalReg.y * 0.5));

    cursorPos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(cursorPos,
                                              float2(cursorPos.x + avalReg.x * 0.5, cursorPos.y + ImGui::GetFontSize()),
                                              ImColor(imstyle.Colors[ImGuiCol_MenuBarBg])); 
    
    ImGui::Text("Compile output");

    cursorPos = ImGui::GetCursorScreenPos();
    float2 logReg = ImGui::GetContentRegionAvail();

    ImGui::GetWindowDrawList()->AddRectFilled(cursorPos,
                                              float2(cursorPos.x + avalReg.x * 0.5, cursorPos.y + logReg.y),
                                              ImColor(20, 20, 35, 128));

    uint32 msgIdx = 0;
    for(auto msgIt = data->logMessages.begin(); msgIt != data->logMessages.end(); msgIt++, msgIdx++)
    {
      float2 cursorPosBeforeWc = ImGui::GetCursorPos();
      float2 cursorPosBefore = ImGui::GetCursorScreenPos();
      ImGui::PushStyleColor(ImGuiCol_Text, float4(0.0f, 0.0f, 0.0f, 0.0f));
      ImGui::Text("%s", msgIt->c_str());
      ImGui::PopStyleColor();
      float2 cursorPosAfter = ImGui::GetCursorScreenPos();

      ImGui::GetWindowDrawList()->AddRectFilled(cursorPosBefore,
                                                float2(cursorPosBefore.x + avalReg.x * 0.5, cursorPosAfter.y),
                                                msgIdx % 2 == 0 ? ImColor(20, 20, 35, 255) : ImColor(10, 10, 10, 255));
      

      ImGui::SetCursorPos(cursorPosBeforeWc);
      
      ImGui::TextColored("%s", msgIt->c_str());
    }
    
    ImGui::EndChild();
  }
  ImGui::EndChild();

  ImGui::PopStyleVar();

  // --------------------------------------------------------------------------
  // New save name popup
  // --------------------------------------------------------------------------  
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
  
}

void scriptFunctionSettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{

}

void saveFunction(ScriptFunctionSettingsWindowData* windowData)
{
  Asset* assetToSave = windowData->function;
  AssetPtr prototypeAsset = assetsManagerFindAsset(windowData->saveName);

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
    prototypeAsset = AssetPtr(scriptFunctionClone(assetToSave));
    assetSetName(prototypeAsset, windowData->saveName);
    assetsManagerAddAsset(prototypeAsset);
  }

  LOG_INFO("Script function '%s' was saved with name '%s'",
           assetGetName(assetToSave).c_str(),
           windowData->saveName);
}
