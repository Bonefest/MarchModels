#include <memory_manager.h>
#include <assets/assets_manager.h>
#include <assets/script_function.h>


#include "editor.h"
#include "ui_utils.h"
#include "ui_styles.h"
#include "assets_list_window.h"
#include "scene_hierarchy_window.h"
#include "script_function_settings_window.h"

struct SceneHierarchyData
{
  bool showMetaInfo = true;
  
  bool listGeometry = true;
  bool listGeometrySDF = true;
  bool listGeometryIDF = true;
  bool listGeometryODF = true;
  bool listGeometryMaterial = true;
  
  bool listLights = true;
  
};

static void sceneHierarchyProcessGeometryArray(Window* window,
                                               std::vector<Geometry*>& array,
                                               SceneHierarchyData* data,
                                               Scene* currentScene);

static void pushCommonButtonsStyle();
static void popCommonButtonsStyle();

static bool8 sceneHierarchyInitialize(Window* window)
{
  return TRUE;
}

static void sceneHierarchyShutdown(Window* window)
{

}

static void sceneHierarchyUpdate(Window* window, float64 delta)
{

}

static Asset* createNewScriptFunction(const std::string& name)
{
  Asset* sfPrototype = assetsManagerFindAsset(name);
  assert(sfPrototype != nullptr);
  
  Asset* sf = scriptFunctionClone(sfPrototype);
  return sf;
}

static void onScriptFunctionIsSelected(Window* window, Asset* asset, uint32 index, void* target)
{
  Asset* targetAsset = (Asset*)target;
  scriptFunctionCopy(targetAsset, asset);
}

static Asset* createNewSDF()
{
  return createNewScriptFunction("sphereSDF");
}

static Asset* createNewIDF()
{
  return createNewScriptFunction("emptyIDF");
}

static Asset* createNewODF()
{
  return createNewScriptFunction("emptyODF");  
}

static Geometry* createNewGeometry()
{

  Geometry* newGeometry;
  assert(createGeometry("sphere", &newGeometry));
  geometrySetSDF(newGeometry, createNewSDF());

  return newGeometry;
}


static bool8 sceneHierarchyDrawGeometryData(Window* window,
                                           Geometry* geometry,
                                           SceneHierarchyData* data,
                                           Scene* currentScene)
{
  const uint32 maxNameSize = 128;
  static char newName[maxNameSize];
  const char* geometryName = geometryGetName(geometry).c_str();
  ImGuiStyle& style = ImGui::GetStyle();  
  
  ImGui::PushID(geometry);

  bool treeOpen = ImGui::TreeNode(geometryName);
  bool8 processedNormally = TRUE;
  
  ImGui::SameLine();
  pushCommonButtonsStyle();

    // Geometry-related action buttons ----------------------------------------
    if(ImGui::SmallButton(ICON_KI_PENCIL"##GeometryChangeName"))
    {
      ImGui::OpenPopup("Change geometry name");
      strcpy(newName, geometryName);
    }

    popCommonButtonsStyle();
      if(textInputPopup("Change geometry name", "Enter a new name", newName, maxNameSize) == TRUE)
      {
        geometrySetName(geometry, newName);
      }
    pushCommonButtonsStyle();

    ImGui::SameLine();       
    ImGui::SmallButton(ICON_KI_LIST"##GeometryChoose");
    
    ImGui::SameLine();
    ImGui::SmallButton(ICON_KI_COG"##GeometryEdit");
    
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, (float4)DeleteClr);
      if(ImGui::SmallButton(ICON_KI_TRASH"##GeometryRemove"))
      {
        processedNormally = FALSE;
      }
    ImGui::PopStyleColor();

    // Geometry content -------------------------------------------------------
    if(treeOpen)
    {

      // Creation buttons -----------------------------------------------------
      if(data->showMetaInfo == TRUE)
      {
        ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);

          if(geometryIsLeaf(geometry) == TRUE && geometryHasSDF(geometry) == FALSE)
          {
            if(ImGui::SmallButton("[New SDF]"))
            {
              geometrySetSDF(geometry, createNewSDF());
            }
            ImGui::SameLine();
          }

          if(ImGui::SmallButton("[New IDF]"))
          {
            geometryAddIDF(geometry, createNewIDF());
          }

          ImGui::SameLine();
          if(ImGui::SmallButton("[New ODF]"))
          {
            geometryAddODF(geometry, createNewODF());
          }

          ImGui::SameLine();
          if(ImGui::SmallButton("[New child]"))
          {
            geometryAddChild(geometry, createNewGeometry());
          }

        ImGui::PopStyleColor();
      }
      
      // Script functions -----------------------------------------------------

      std::vector<Asset*> functions = geometryGetScriptFunctions(geometry);

      for(Asset* function: functions)
      {
        ScriptFunctionType type = scriptFunctionGetType(function);
        const char* functionTypeLabel;

        if(type == SCRIPT_FUNCTION_TYPE_SDF)
        {
          if(!data->listGeometrySDF)
          {
            continue;
          }

          functionTypeLabel = "SDF";
        }
        else if(type == SCRIPT_FUNCTION_TYPE_IDF)
        {
          if(!data->listGeometryIDF)
          {
            continue;
          }

          functionTypeLabel = "IDF";
        }
        else if(type == SCRIPT_FUNCTION_TYPE_ODF)
        {
          if(!data->listGeometryODF)
          {
            continue;
          }

          functionTypeLabel = "ODF";
        }

        ImGui::PushID(function);
          
          ImGui::TextColored("_<C>0x4bcc4bff</C>_[%s] _<C>0x1</C>_'%s'",
                             functionTypeLabel,
                             assetGetName(function).c_str());

          ImGui::SameLine();
          if(ImGui::SmallButton(ICON_KI_LIST))
          {
            float2 itemTopPos = ImGui::GetItemRectMin();
            
            Window* assetsListWindow = windowManagerGetWindow(assetsListWindowGetIdentifier());
            if(assetsListWindow != nullptr)
            {
              windowManagerRemoveWindow(assetsListWindow, TRUE);
            }

            std::vector<Asset*> assetsToDisplay = assetsManagerGetAssetsByType(ASSET_TYPE_SCRIPT_FUNCTION);

            // If list button was clicked for a SDF, only SDFs should be displayed - other assets should
            // be removed.
            
            auto removeIt = std::remove_if(assetsToDisplay.begin(),
                                           assetsToDisplay.end(),
                                           [function](Asset* asset) {
                                             return scriptFunctionGetType(asset) != scriptFunctionGetType(function);
                                           });
            
            assetsToDisplay.erase(removeIt, assetsToDisplay.end());

            createAssetsListWindowWithSomeAssets(assetsToDisplay, &assetsListWindow);
            assetsListWindowSetSelectCallback(assetsListWindow, onScriptFunctionIsSelected, function);
            windowSetSize(assetsListWindow, float2(180.0, 100.0));
            windowSetPosition(assetsListWindow, itemTopPos + float2(10, 10));
            windowSetFocused(assetsListWindow, TRUE);
            windowManagerAddWindow(assetsListWindow);
          }

          ImGui::SameLine();        
          if(ImGui::SmallButton(ICON_KI_COG))
          {
              Window* scriptFunctionSettingsWindow = nullptr;
              if(windowManagerHasWindow(scriptFunctionWindowIdentifier(function)) == FALSE)
              {
                assert(createScriptFunctionSettingsWindow(function, &scriptFunctionSettingsWindow));
                windowSetSize(scriptFunctionSettingsWindow, float2(640.0f, 360.0f));
                windowManagerAddWindow(scriptFunctionSettingsWindow);
              }
          }

          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Text, (float4)DeleteClr);
            if(ImGui::SmallButton(ICON_KI_TRASH))
            {
              // If settings window is opened - close it manually (otherwise it will use dangling pointer)
              if(windowManagerHasWindow(scriptFunctionWindowIdentifier(function)) == TRUE)
              {
                Window* scriptFunctionSettingsWindow = windowManagerGetWindow(scriptFunctionWindowIdentifier(function));
                windowClose(scriptFunctionSettingsWindow);
              }
              
              geometryRemoveFunction(geometry, function);            
              destroyAsset(function);
            }
          ImGui::PopStyleColor();

        ImGui::PopID();
      }
      
      // Children geometry ----------------------------------------------------
      popCommonButtonsStyle();
        std::vector<Geometry*>& children = geometryGetChildren(geometry);
        sceneHierarchyProcessGeometryArray(window, children, data, currentScene);
      pushCommonButtonsStyle();
      ImGui::TreePop();
    }

  popCommonButtonsStyle();

  ImGui::PopID();

  return processedNormally;
}

static void sceneHierarchyDraw(Window* window, float64 delta)
{
  Scene* currentScene = editorGetCurrentScene();
  SceneHierarchyData* data = (SceneHierarchyData*)windowGetInternalData(window);
  
  char _tempBuf[73]{};
  ImGui::InputTextWithHint("##SceneHierarchySearch", ICON_KI_SEARCH" Search name", _tempBuf, 73);
  ImGui::SameLine();
  if(ImGui::Button("Search##SceneHierarchySearch"))
  {

  }

  ImGui::SameLine();
  if(ImGui::Button("Filter##SceneHierarchySearch"))
  {
    ImGui::OpenPopup("Filter popup##SceneHierarchySearch");
  }

  if(ImGui::BeginPopup("Filter popup##SceneHierarchySearch"))
  {
    ImGui::Checkbox("List geometry", &data->listGeometry);
    if(data->listGeometry)
    {
      ImGui::Indent();
      
      ImGui::Checkbox("List SDFs", &data->listGeometrySDF);
      ImGui::Checkbox("List IDFs", &data->listGeometryIDF);
      ImGui::Checkbox("List ODFs", &data->listGeometryODF);
      ImGui::Checkbox("List materials", &data->listGeometryMaterial);

      ImGui::Unindent();
    }

    ImGui::Checkbox("List lights", &data->listLights);
    ImGui::Checkbox("Show metainfo", &data->showMetaInfo);    

    ImGui::EndPopup();
  }

  if(data->showMetaInfo == TRUE)
  {
    pushCommonButtonsStyle();
    ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);
      if(ImGui::SmallButton("[New geometry]"))
      {
        sceneAddGeometry(currentScene, createNewGeometry());
      }

      ImGui::SameLine();
      ImGui::SmallButton("[New light]");  
    ImGui::PopStyleColor();
    popCommonButtonsStyle();

    ImGui::Separator();    
  }
  
  if(data->listGeometry)
  {
    std::vector<Geometry*>& geometryArray = sceneGetGeometry(currentScene);
    sceneHierarchyProcessGeometryArray(window, geometryArray, data, currentScene);
  }
}

static void sceneHierarchyProcessInput(Window* window, const EventData& eventData, void* sender)
{

}

bool8 createSceneHierarchyWindow(const std::string& identifier, Window** outWindow)
{
  WindowInterface interface = {};
  interface.initialize = sceneHierarchyInitialize;
  interface.shutdown = sceneHierarchyShutdown;
  interface.update = sceneHierarchyUpdate;
  interface.draw = sceneHierarchyDraw;
  interface.processInput = sceneHierarchyProcessInput;

  if(allocateWindow(interface, identifier, outWindow) == FALSE)
  {
    return FALSE;
  }
  
  SceneHierarchyData* data = engineAllocObject<SceneHierarchyData>(MEMORY_TYPE_GENERAL);
  windowSetInternalData(*outWindow, data);
  
  return TRUE;
}

void sceneHierarchyProcessGeometryArray(Window* window,
                                        std::vector<Geometry*>& array,
                                        SceneHierarchyData* data,
                                        Scene* currentScene)
{
  for(auto geometryIt = array.begin(); geometryIt != array.end();)
  {
    // If after processing a geometry it has returned false, then a removing command was requested -
    // destroy the geometry and remove it from the array
    if(sceneHierarchyDrawGeometryData(window, *geometryIt, data, currentScene) == FALSE)
    {
      destroyGeometry(*geometryIt);
      geometryIt = array.erase(geometryIt);
    }
    else
    {
      geometryIt++;
    }
  }  
}

void pushCommonButtonsStyle()
{
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, float2(0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_Button, (float4)ImColor(0, 0, 0, 0));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (float4)ImColor(0, 0, 0, 0));  
}

void popCommonButtonsStyle()
{
  ImGui::PopStyleColor(2);
  ImGui::PopStyleVar();
}
