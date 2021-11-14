#include <memory_manager.h>
#include <assets/assets_manager.h>
#include <assets/script_function.h>

#include "editor.h"
#include "ui_utils.h"
#include "ui_styles.h"
#include "list_window.h"
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

static AssetPtr createNewScriptFunction(const std::string& name)
{
  AssetPtr sfPrototype = assetsManagerFindAsset(name);
  assert(sfPrototype != nullptr);

  return AssetPtr(scriptFunctionClone(sfPrototype));
}

static void onScriptFunctionIsSelected(Window* window, void* selection, uint32 index, void* target)
{
  Asset* selectedAsset = (Asset*)selection;
  Asset* targetAsset = (Asset*)target;
  scriptFunctionCopy(targetAsset, selectedAsset);
}

static AssetPtr createNewSDF()
{
  return createNewScriptFunction("sphereSDF");
}

static AssetPtr createNewIDF()
{
  return createNewScriptFunction("emptyIDF");
}

static AssetPtr createNewODF()
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
  const char* geometryName = geometryGetName(geometry).c_str();
  ImGuiStyle& style = ImGui::GetStyle();  
  
  ImGui::PushID(geometry);

  bool treeOpen = ImGui::TreeNode(geometryName);
  bool8 processedNormally = TRUE;
  
  ImGui::SameLine();
  pushIconButtonStyle();

    // Geometry-related action buttons ----------------------------------------
    if(ImGui::SmallButton(ICON_KI_PENCIL"##GeometryChangeName"))
    {
      ImGui::OpenPopup("Change geometry name");
      strcpy(textInputPopupGetBuffer(), geometryName);
    }

    popIconButtonStyle();
      ImGuiUtilsButtonsFlags pressedButton = textInputPopup("Change geometry name", "Enter a new name");

      if(ImGuiUtilsButtonsFlags_Accept == pressedButton)
      {
        geometrySetName(geometry, textInputPopupGetBuffer());
      }
    pushIconButtonStyle();

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

      std::vector<AssetPtr> functions = geometryGetScriptFunctions(geometry);

      for(AssetPtr function: functions)
      {
        ScriptFunctionType type = scriptFunctionGetType(function);

        if(type == SCRIPT_FUNCTION_TYPE_SDF && !data->listGeometrySDF)
        {
            continue;
        }
        else if(type == SCRIPT_FUNCTION_TYPE_IDF && !data->listGeometryIDF)
        {
          continue;
        }
        else if(type == SCRIPT_FUNCTION_TYPE_ODF && !data->listGeometryODF)
        {
          continue;
        }

        const char* functionTypeLabel = scriptFunctionTypeLabel(type);
        
        ImGui::PushID(function);
          
          ImGui::TextColored("_<C>0x4bcc4bff</C>_[%s] _<C>0x1</C>_'%s'",
                             functionTypeLabel,
                             assetGetName(function).c_str());

          ImGui::SameLine();
          if(ImGui::SmallButton(ICON_KI_LIST))
          {
            float2 itemTopPos = ImGui::GetItemRectMin();
            
            WindowPtr prevAssetsListWindow = windowManagerGetWindow("Script functions list");
            if(prevAssetsListWindow != nullptr)
            {
              windowManagerRemoveWindow(prevAssetsListWindow);
            }

            std::vector<AssetPtr> assetsToDisplay = assetsManagerGetAssetsByType(ASSET_TYPE_SCRIPT_FUNCTION);

            // NOTE: Remove all script functions that don't have similar type
            auto removeIt = std::remove_if(assetsToDisplay.begin(),
                                           assetsToDisplay.end(),
                                           [function](AssetPtr asset) {
                                             return scriptFunctionGetType(asset) != scriptFunctionGetType(function);
                                           });
            
            assetsToDisplay.erase(removeIt, assetsToDisplay.end());

            std::vector<ListItem> items = {};
            for(AssetPtr asset: assetsToDisplay)
            {
              items.push_back(ListItem{assetGetName(asset), asset});
            }

            
            Window* assetsListWindow;
            assert(createListWindow("Script functions list",
                                    "Select a function",
                                    items,
                                    onScriptFunctionIsSelected,
                                    function,
                                    &assetsListWindow));
            
            listWindowSetCloseOnLoseFocus(assetsListWindow, TRUE);
            windowSetSize(assetsListWindow, float2(180.0, 100.0));
            windowSetPosition(assetsListWindow, itemTopPos + float2(10, 10));
            windowSetFocused(assetsListWindow, TRUE);
            
            windowManagerAddWindow(WindowPtr(assetsListWindow));
          }

          ImGui::SameLine();        
          if(ImGui::SmallButton(ICON_KI_COG))
          {
              Window* scriptFunctionSettingsWindow = nullptr;
              if(windowManagerHasWindow(scriptFunctionWindowIdentifier(function)) == FALSE)
              {
                assert(createScriptFunctionSettingsWindow(function, &scriptFunctionSettingsWindow));
                windowSetSize(scriptFunctionSettingsWindow, float2(640.0f, 360.0f));
                windowManagerAddWindow(WindowPtr(scriptFunctionSettingsWindow));
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
      popIconButtonStyle();
        std::vector<Geometry*>& children = geometryGetChildren(geometry);
        sceneHierarchyProcessGeometryArray(window, children, data, currentScene);
      pushIconButtonStyle();
      ImGui::TreePop();
    }

    popIconButtonStyle();

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
    pushIconButtonStyle();
    ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);
      if(ImGui::SmallButton("[New geometry]"))
      {
        sceneAddGeometry(currentScene, createNewGeometry());
      }

      ImGui::SameLine();
      ImGui::SmallButton("[New light]");  
    ImGui::PopStyleColor();
    popIconButtonStyle();

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
