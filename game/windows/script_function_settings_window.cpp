#include <deque>
#include <vector>
#include <string>

using std::deque;
using std::vector;
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

struct ArgumentDesc
{
  const char* name;
  const char* description;
};

static const vector<ArgumentDesc> builtinArgsDescs =
{
  {"p", "(IDF/SDF only) incoming point in world space"},
  {"d", "(ODF only) outcoming distance"},
  {"d1, d2", "(PCF only) outcoming distances to combine"},
  
  {"params.time", "current time"},
  {"params.gamma", "power used for gamma-correction decoding"},
  {"params.invGamma", "power used for gamma-correction encoding"},
  {"params.pixelGapX", "horizontal gap size between pixels"},
  {"params.pixelGapY", "vertical gap size between pixels"},
  {"params.resolution", "film resolution"}, 
  {"params.invResolution", "inversed film resolution"}, 
  {"params.gapResolution", "resolution with applied gap between pixels"},
  {"params.invGapResolution", "inversed resolution with applied gap between pixels"},
  
  {"params.camPosition", "camera position in world space"},
  {"params.camOrientation", "camera orientation in world space expressed as a quaternion"},
  {"params.camFwdAxis", "camera forward z axis in world space"},
  {"params.camSideAxis", "camera side x axis in world space"},
  {"params.camUpAxis", "camera up y axis in world space"},    
  {"params.camNDCCameraMat", "NDC->Camera space transformation matrix"},
  {"params.camCameraNDCMat", "Camera->NDC space transformation matrix"},
  {"params.camNDCWorldMat", "NDC->World space transformation matrix"},
  {"params.camWorldNDCMat", "World->NDC space transformation matrix"},
  {"params.camCameraWorldMat", "Camera->World space transformation matrix"},
  {"params.camWorldCameraMat", "World->Camera space transformation matrix"},

  {"geometryID", "Geometry ID"},
  {"geo.position", "Geometry position"},
  {"geo.geoWorldMat", "Geometry->World space transformation matrix"},
  {"geo.worldGeoMat", "World->Geometry space transformation matrix"},
  {"geo.geoParentMat", "Geometry->Parent space transformation matrix"},
  {"geo.parentGetoMat", "Parent->Geometry space transformation matrix"}
};

struct ScriptFunctionSettingsWindowData
{
  char saveName[255];
  char newArgName[255];
  char codeBuf[4096];
  Logger* logger;
  
  AssetPtr owner;
  AssetPtr function;

  AssetPtr dummyRootGeometry;
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

  Asset* dummyRootGeometry;
  createGeometry("dummyRoot", &dummyRootGeometry);
  data->dummyRootGeometry = AssetPtr(dummyRootGeometry);
  
  Asset* dummyGeometry;
  createGeometry("dummy1", &dummyGeometry);
  geometrySetAABBAutomaticallyCalculated(dummyGeometry, FALSE);
  geometryAddChild(data->dummyRootGeometry, AssetPtr(dummyGeometry));
  
  createGeometry("dummy2", &dummyGeometry);
  geometrySetAABBAutomaticallyCalculated(dummyGeometry, FALSE);
  data->dummyGeometry = AssetPtr(dummyGeometry);
  
  geometryAddChild(data->dummyRootGeometry, data->dummyGeometry);
  
  if(createLogger(32, FALSE, FALSE, TRUE, nullptr, &data->logger) == FALSE)
  {
    return FALSE;
  }
  
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
    destroyLogger(data->logger);
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
  ImGui::SmallButton("[" ICON_KI_DOWNLOAD " Open]");
  ImGui::SameLine();
  ImGui::PopStyleColor();

  // --------------------------------------------------------------------------
  // Save button
  // --------------------------------------------------------------------------
  ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);    
  if(ImGui::SmallButton("[" ICON_KI_UPLOAD " Save]"))
  {
    saveFunction(data);
  }
  ImGui::SameLine();
  ImGui::PopStyleColor();  

  // --------------------------------------------------------------------------
  // Save as button
  // --------------------------------------------------------------------------
  ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);    
  if(ImGui::SmallButton("[" ICON_KI_UPLOAD " Save as]"))
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

        logMsg(data->logger, LOG_MESSAGE_TYPE_SUCCESS, "Forked to an instance");
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

      logMsg(data->logger, LOG_MESSAGE_TYPE_SUCCESS, "Unforked back to the prototype");      
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
  if(ImGui::SmallButton(typeButton) && isPrototype == TRUE && sfType != SCRIPT_FUNCTION_TYPE_PCF)
  {
    sfType = (ScriptFunctionType)(((int32)sfType + 1) % (int32)SCRIPT_FUNCTION_TYPE_COUNT);

    // NOTE: PCF cannot be selected
    if(sfType == SCRIPT_FUNCTION_TYPE_PCF)
    {
      sfType = (ScriptFunctionType)(((int32)sfType + 1) % (int32)SCRIPT_FUNCTION_TYPE_COUNT);      
    }
    
    scriptFunctionSetType(data->function, sfType);

    logMsg(data->logger,
           LOG_MESSAGE_TYPE_SUCCESS,
           "changed script function type to '%s'", scriptFunctionTypeLabel(sfType));
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
  if(ImGui::SmallButton(ICON_KI_WRENCH " Compile "))
  {
    AssetPtr targetGeometry = (sfType == SCRIPT_FUNCTION_TYPE_PCF ? data->dummyRootGeometry : data->dummyGeometry);

    std::string previousCode = scriptFunctionGetRawCode(data->function);
    scriptFunctionSetCode(data->function, data->codeBuf);
    
    geometryRemoveFunction(targetGeometry, data->function);
    geometryAddFunction(targetGeometry, data->function);

    geometryUpdate(data->dummyRootGeometry, 0.0f);
    bool8 compilationSucceeded = geometryGetDrawProgram(targetGeometry) != nullptr ? TRUE : FALSE;
    
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

    if(compilationSucceeded == TRUE)
    {
      logMsg(data->logger, LOG_MESSAGE_TYPE_SUCCESS, "Compilation has succeeded.");
    }
    else
    {
      logMsg(data->logger, LOG_MESSAGE_TYPE_ERROR, "Compilation has failed.");
    }
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

        if(ImGui::BeginTable("ScriptFunctionArgsTable",
                             4,
                             ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp))
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
              logMsg(data->logger,
                     LOG_MESSAGE_TYPE_SUCCESS,
                     "Removed argument '%s'",
                     argIt->first.c_str());
              
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


        ImGui::BeginDisabled((argNameAlreadyExists || strlen(data->newArgName) == 0) ? TRUE : FALSE);
        if(ImGui::Button("Register"))
        {
          if(argNameAlreadyExists == FALSE)
          {
            args[data->newArgName] = 0.0f;

            logMsg(data->logger,
                   LOG_MESSAGE_TYPE_SUCCESS,
                   "Registered new argument '%s'",
                   data->newArgName);
          
            data->newArgName[0] = '\0';
          }
          else
          {
            logMsg(data->logger,
                   LOG_MESSAGE_TYPE_ERROR,
                   "Arguments '%s' already exists!",
                   data->newArgName);
          }
        }
        ImGui::EndDisabled();
        

        ImGui::EndTabItem();
      }
      if(ImGui::BeginTabItem("Built-in arguments"))
      {
        if(ImGui::BeginTable("ScriptFunctionBuiltinArgsTable",
                             2,
                             ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp))
        {
          ImGui::TableSetupColumn("Name", 0, 0.5f);
          ImGui::TableSetupColumn("Description", 0, 0.5f);
          ImGui::TableHeadersRow();

          for(auto argIt = builtinArgsDescs.begin(); argIt != builtinArgsDescs.end(); argIt++)
          {
            ImGui::TableNextRow();

            // Name column
            ImGui::TableSetColumnIndex(0);
            ImGui::TextWrapped("%s", argIt->name);

            // Description column
            ImGui::TableSetColumnIndex(1);
            ImGui::TextWrapped("%s", argIt->description);
          }

          ImGui::EndTable();           
        }
        
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

    const std::deque<LogMessage> logMessages = logGetMessages(data->logger);
    uint32 msgIdx = 0;
    for(auto msgIt = logMessages.rbegin(); msgIt != logMessages.rend(); msgIt++, msgIdx++)
    {
      float2 cursorPosBeforeWc = ImGui::GetCursorPos();
      float2 cursorPosBefore = ImGui::GetCursorScreenPos();
      ImGui::PushStyleColor(ImGuiCol_Text, (float4)InvisibleClr);
        ImGui::TextWrapped("%s", msgIt->message.c_str());
      ImGui::PopStyleColor();
      float2 cursorPosAfter = ImGui::GetCursorScreenPos();

      ImGui::GetWindowDrawList()->AddRectFilled(cursorPosBefore,
                                                float2(cursorPosBefore.x + avalReg.x * 0.5, cursorPosAfter.y),
                                                msgIdx % 2 == 0 ? ImColor(20, 20, 35, 255) : ImColor(10, 10, 10, 255));
      

      ImGui::SetCursorPos(cursorPosBeforeWc);

      ImGui::PushStyleColor(ImGuiCol_Text, (float4)logTypeToClr(msgIt->type));
        ImGui::TextWrapped("%s", msgIt->message.c_str());
      ImGui::PopStyleColor();
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

  logMsg(windowData->logger,
         LOG_MESSAGE_TYPE_SUCCESS,         
         "Saved with name '%s'",
         windowData->saveName);
}
