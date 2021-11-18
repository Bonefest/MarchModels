#include <memory_manager.h>
#include <assets/assets_manager.h>
#include <assets/script_function.h>

#include "editor.h"
#include "ui_utils.h"
#include "ui_styles.h"
#include "list_window.h"
#include "scene_hierarchy_window.h"
#include "geometry_settings_window.h"
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
                                               std::vector<AssetPtr>& array,
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
  
  return sfPrototype;
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

static AssetPtr createNewGeometry()
{

  Asset* newGeometry;
  assert(createGeometry("sphere", &newGeometry));
  geometrySetSDF(newGeometry, createNewSDF());

  return AssetPtr(newGeometry);
}


static bool8 sceneHierarchyDrawGeometryData(Window* window,
                                           AssetPtr geometry,
                                           SceneHierarchyData* data,
                                           Scene* currentScene)
{
  const char* geometryName = assetGetName(geometry).c_str();
  ImGuiStyle& style = ImGui::GetStyle();  
  
  ImGui::PushID(geometry);

  bool treeOpen = ImGui::TreeNode(geometryName);
  bool8 processedNormally = TRUE;
  
  ImGui::SameLine();
  pushIconSmallButtonStyle();

    // Geometry-related action buttons ----------------------------------------
    if(ImGui::SmallButton(ICON_KI_PENCIL"##GeometryChangeName"))
    {
      ImGui::OpenPopup("Change geometry name");
      strcpy(textInputPopupGetBuffer(), geometryName);
    }

    popIconSmallButtonStyle();
      ImGuiUtilsButtonsFlags pressedButton = textInputPopup("Change geometry name", "Enter a new name");

      if(ImGuiUtilsButtonsFlags_Accept == pressedButton)
      {
        assetSetName(geometry, textInputPopupGetBuffer());
      }
    pushIconSmallButtonStyle();

    ImGui::SameLine();       
    ImGui::SmallButton(ICON_KI_LIST"##GeometryChoose");
    
    ImGui::SameLine();
    if(ImGui::SmallButton(ICON_KI_COG"##GeometryEdit"))
    {
      WindowPtr geometrySettingsWindow = windowManagerGetWindow(geometrySettingsWindowIdentifier(geometry));
      if(geometrySettingsWindow == nullptr)
      {
        Window* newSettingsWindow = nullptr;        
        assert(createGeometrySettingsWindow(currentScene, geometry, &newSettingsWindow));
        windowSetSize(newSettingsWindow, float2(480.0f, 180.0f));
        windowManagerAddWindow(WindowPtr(newSettingsWindow));
      }
      else
      {
        windowSetFocused(geometrySettingsWindow, TRUE);
      }      
    }
    
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

      popIconSmallButtonStyle();
      
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

          drawScriptFunctionItem(geometry, function);
        }

      // Children geometry ----------------------------------------------------

        std::vector<AssetPtr>& children = geometryGetChildren(geometry);
        sceneHierarchyProcessGeometryArray(window, children, data, currentScene);
        
      pushIconSmallButtonStyle();
      ImGui::TreePop();
    }

    popIconSmallButtonStyle();

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
    pushIconSmallButtonStyle();
    ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);
      if(ImGui::SmallButton("[New geometry]"))
      {
        sceneAddGeometry(currentScene, createNewGeometry());
      }

      ImGui::SameLine();
      ImGui::SmallButton("[New light]");  
    ImGui::PopStyleColor();
    popIconSmallButtonStyle();

    ImGui::Separator();    
  }
  
  if(data->listGeometry)
  {
    std::vector<AssetPtr>& geometryArray = sceneGetGeometry(currentScene);
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
                                        std::vector<AssetPtr>& array,
                                        SceneHierarchyData* data,
                                        Scene* currentScene)
{
  for(auto geometryIt = array.begin(); geometryIt != array.end();)
  {
    // If after processing a geometry it has returned false, then a removing command was requested -
    // destroy the geometry and remove it from the array
    if(sceneHierarchyDrawGeometryData(window, *geometryIt, data, currentScene) == FALSE)
    {
      geometryIt = array.erase(geometryIt);
    }
    else
    {
      geometryIt++;
    }
  }  
}
