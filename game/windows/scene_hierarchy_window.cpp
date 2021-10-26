#include <memory_manager.h>

#include "editor.h"
#include "scene_hierarchy_window.h"

struct SceneHierarchyData
{
  bool listGeometry = true;
  bool listGeometrySDF = true;
  bool listGeometryIDF = true;
  bool listGeometryODF = true;
  bool listGeometryMaterial = true;
  
  bool listLights = true;
  
};

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

static void sceneHierarchyDrawGeometryList(Window* window, Geometry* geometry)
{
  bool treeOpen = ImGui::TreeNode(geometryGetName(geometry).c_str());

  ImGui::SameLine();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, float2(0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_Button, (float4)ImColor(0, 0, 0, 0));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (float4)ImColor(0, 0, 0, 0));  
  
    ImGui::SmallButton(ICON_KI_TRASH"##GeometryRemove");
    ImGui::SameLine();
    
    ImGui::SmallButton(ICON_KI_COG"##GeometryEdit");
    ImGui::SameLine();
    
    ImGui::SmallButton(ICON_KI_GRID"##GeometryChoose");
    ImGui::SameLine();
    
    ImGui::SmallButton(ICON_KI_PLUS_CIRCLE"##GeometryCreate"); // TODO: Is not suitable here, should be on the main tab (allows us to add new geometry)
    
  ImGui::PopStyleColor(2);
  ImGui::PopStyleVar();

  if(treeOpen)
  {
    ImGui::TreePop();
  }
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

    ImGui::EndPopup();
  }

  if(data->listGeometry)
  {
    const std::vector<Geometry*> geometryArray = sceneGetGeometry(currentScene);
    for(Geometry* geometry: geometryArray)
    {
      sceneHierarchyDrawGeometryList(window, geometry);
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
