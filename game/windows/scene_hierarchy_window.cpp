#include <vector>

#include <utils.h>
#include <memory_manager.h>
#include <assets/assets_manager.h>
#include <assets/script_function.h>

#include "editor.h"
#include "ui_utils.h"
#include "ui_styles.h"
#include "list_window.h"
#include "editor_utils.h"
#include "scene_hierarchy_window.h"
#include "geometry_settings_window.h"
#include "script_function_settings_window.h"

using std::vector;

struct SceneHierarchyData
{
  bool showMetaInfo = true;
  
  bool listGeometrySDF = true;
  bool listGeometryIDF = true;
  bool listGeometryODF = true;
  bool listGeometryPCF = true;
  bool listGeometryMaterial = true;

  vector<AssetWPtr> selectedGeometry;
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

static void sceneHierarchyDrawHeader(const char* objectTypeName,
                                     const char* objectTypeNamePlural,
                                     uint32 selectedObjectsCount,
                                     float32& newButtonWidth,
                                     bool& clearPressed,
                                     bool& newPressed,
                                     bool& headerOpened)
{
  ImGuiStyle& imstyle = ImGui::GetStyle();  
  float32 headerAvalSize = ImGui::GetContentRegionAvail().x;

  char headerTitle[32], newButtonTitle[32];
  sprintf(headerTitle, "Scene %s tree", objectTypeNamePlural);
  sprintf(newButtonTitle, "[New %s]", objectTypeName);  
  
  ImGui::PushStyleColor(ImGuiCol_Header, (float4)imstyle.Colors[ImGuiCol_MenuBarBg]);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, (float4)imstyle.Colors[ImGuiCol_MenuBarBg] * 1.1f);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, (float4)imstyle.Colors[ImGuiCol_MenuBarBg] * 1.2f);
  headerOpened = ImGui::CollapsingHeader(headerTitle, ImGuiTreeNodeFlags_AllowItemOverlap);
  ImGui::PopStyleColor(3);

  if(selectedObjectsCount > 0)
  {
    ImGui::SameLine();
    ImGui::TextColored("_<C>%#010x</C>_ (", revbytes((uint32)DarkFadedClr));
    ImGui::SameLine();

    pushIconSmallButtonStyle();
    ImGui::PushStyleColor(ImGuiCol_Text, (float4)DangerClr);
    clearPressed = ImGui::SmallButton("X");
    ImGui::PopStyleColor();
    popIconSmallButtonStyle();
    ImGui::SameLine();
    
    ImGui::TextColored("_<C>%#010x</C>_Selected %u element%s)",
                       revbytes((uint32)DarkFadedClr),
                       selectedObjectsCount,
                       selectedObjectsCount == 1 ? "" : "s");

  }

  pushIconSmallButtonStyle();
  ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);
  ImGui::SameLine(headerAvalSize - newButtonWidth);  
  newPressed = ImGui::SmallButton(newButtonTitle);
  newButtonWidth = ImGui::GetItemRectSize().x;

  ImGui::PopStyleColor();
  popIconSmallButtonStyle();
}

static bool8 sceneHierarchyDrawGeometryData(Window* window,
                                            AssetPtr geometry,
                                            SceneHierarchyData* data,
                                            Scene* currentScene)
{
  const char* geometryName = assetGetName(geometry).c_str();
  ImGuiStyle& style = ImGui::GetStyle();  
  
  ImGui::PushID(geometryGetID(geometry));

  bool treeSelected = geometryIsSelected(geometry) ? TRUE : FALSE;
  ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
  if(treeSelected)
  {
    treeFlags |= ImGuiTreeNodeFlags_Selected;
  }

  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, float4(0.5f, 0.5f, 0.5f, 0.25f));
    bool treeOpen = ImGui::TreeNodeEx(geometryName, treeFlags);
  ImGui::PopStyleColor();
  bool treeClicked = ImGui::IsItemClicked();
  bool ctrlPressed = ImGui::GetIO().KeyCtrl;
  
  bool8 processedNormally = TRUE;
  
  ImGui::SameLine();
  pushIconSmallButtonStyle();

    // Geometry-related action buttons ----------------------------------------
    popIconSmallButtonStyle();
      processedNormally = drawGeometryItemActionButtons(currentScene, geometry) == TRUE ? FALSE : TRUE;
    pushIconSmallButtonStyle();
      
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
              geometryAddFunction(geometry, createDefaultSDF());
            }
            ImGui::SameLine();
          }

          if(ImGui::SmallButton("[New IDF]"))
          {
            geometryAddFunction(geometry, createDefaultIDF());
          }

          ImGui::SameLine();
          if(ImGui::SmallButton("[New ODF]"))
          {
            geometryAddFunction(geometry, createDefaultODF());
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
          else if(type == SCRIPT_FUNCTION_TYPE_PCF && !data->listGeometryPCF)
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

    if(treeClicked)
    {
      if(!ctrlPressed)
      {
        editorClearSelectedGeometry();
      }

      geometrySetSelected(geometry, treeSelected ? FALSE : TRUE);      
    }

    popIconSmallButtonStyle();

  ImGui::PopID();

  return processedNormally;
}

static bool8 sceneHierarchyDrawLightSourceData(Window* window,
                                               AssetPtr lightSource,
                                               SceneHierarchyData* data,
                                               Scene* currentScene)
{
  const char* lightSourceName = assetGetName(lightSource).c_str();
  bool8 removeIsPressed = FALSE;
  
  ImGui::PushID(lightSource.raw());
  
    ImGui::TreeNodeEx(lightSourceName, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
    ImGui::SameLine();
    removeIsPressed = drawLightSourceItemActionButtons(lightSource);
    
  ImGui::PopID();

  return removeIsPressed;
}

static void sceneHierarchyDraw(Window* window, float64 delta)
{
  ImGuiStyle& imstyle = ImGui::GetStyle();  
  
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
    ImGui::Checkbox("List SDFs", &data->listGeometrySDF);
    ImGui::Checkbox("List IDFs", &data->listGeometryIDF);
    ImGui::Checkbox("List ODFs", &data->listGeometryODF);
    ImGui::Checkbox("List PCFs", &data->listGeometryPCF);      
    ImGui::Checkbox("List materials", &data->listGeometryMaterial);
    ImGui::Checkbox("Show metainfo", &data->showMetaInfo);    

    ImGui::EndPopup();
  }

  ImGui::Separator();    

  // --------------------------------------------------------------------------
  // Geometry list
  // --------------------------------------------------------------------------
  static float32 newGeometryButtonWidth = 0.0f;
  bool clearGeometryPressed = false, newGeometryPressed = false, showGeometryPressed = false;
  std::vector<AssetPtr> selectedGeometry = editorGetSelectedGeometry();

  sceneHierarchyDrawHeader("geometry", "geometries", selectedGeometry.size(),
                           newGeometryButtonWidth,
                           clearGeometryPressed,
                           newGeometryPressed,
                           showGeometryPressed);

  if(clearGeometryPressed)
  {
    editorClearSelectedGeometry();
  }

  if(newGeometryPressed)
  {
    sceneAddGeometry(currentScene, createNewGeometry());
  }

  if(showGeometryPressed)
  {
    ImGui::Indent();
    std::vector<AssetPtr>& geometryArray = sceneGetChildren(currentScene);
    sceneHierarchyProcessGeometryArray(window, geometryArray, data, currentScene);
    ImGui::Unindent();    
  }

  // --------------------------------------------------------------------------
  // Light sources list
  // --------------------------------------------------------------------------
  static float32 newLightSourceButtonWidth = 0.0f;
  bool clearLsourcePressed = false, newLsourcePressed = false, showLsourcePressed = false;
  std::vector<AssetPtr> selectedLightSources = editorGetSelectedLightSources();

  sceneHierarchyDrawHeader("light", "lights", selectedLightSources.size(),
                           newLightSourceButtonWidth,
                           clearLsourcePressed,
                           newLsourcePressed,
                           showLsourcePressed);

  if(clearLsourcePressed)
  {
    editorClearSelectedLightSources();
  }

  if(newLsourcePressed)
  {
    sceneAddLightSource(currentScene, createNewLight());
  }

  if(showLsourcePressed)
  {
    std::vector<AssetPtr>& lightSources = sceneGetLightSources(currentScene);
    for(auto lsourceIt = lightSources.begin(); lsourceIt != lightSources.end();)
    {
      if(sceneHierarchyDrawLightSourceData(window, *lsourceIt, data, currentScene) == TRUE)
      {
        lightSources.erase(lsourceIt);
      }
      else
      {
        lsourceIt++;
      }
    }
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
  std::vector<AssetPtr> geometryToRemove;
  
  for(auto geometryIt = array.begin(); geometryIt != array.end(); geometryIt++)
  {
    // If after processing a geometry it has returned false, then a removing command was requested -
    // destroy the geometry and remove it from the array
    if(sceneHierarchyDrawGeometryData(window, *geometryIt, data, currentScene) == FALSE)
    {
      geometryToRemove.push_back(*geometryIt);
    }
  }

  if(!geometryToRemove.empty())
  {
    AssetPtr parent = geometryGetParent(geometryToRemove.front());
    for(AssetPtr geometry: geometryToRemove)
    {
      geometryRemoveChild(parent, geometry);
    }
  }
}
