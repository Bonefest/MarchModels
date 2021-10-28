#include <memory_manager.h>

#include "editor.h"
#include "imgui_utils.h"
#include "scene_hierarchy_window.h"

struct SceneHierarchyData
{
  bool listGeometry = true;
  bool listGeometrySDF = true;
  bool listGeometryIDF = true;
  bool listGeometryODF = true;
  bool listGeometryMaterial = true;
  bool listGeometryMeta = true;
  
  bool listLights = true;
  
};

static void pushCommonButtonsStyle()
{
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, float2(0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_Button, (float4)ImColor(0, 0, 0, 0));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (float4)ImColor(0, 0, 0, 0));  
}

static void popCommonButtonsStyle()
{
  ImGui::PopStyleColor(2);
  ImGui::PopStyleVar();
}

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

static void sceneHierarchyDrawGeometryList(Window* window, Geometry* geometry, SceneHierarchyData* data)
{
  static char newName[128];
  const char* geometryName = geometryGetName(geometry).c_str();
  
  ImGui::PushID(geometry);
  
  bool treeOpen = ImGui::TreeNode(geometryName);
  ImGui::SameLine();

  ImGuiStyle& style = ImGui::GetStyle();  
  // Geometry header rendering
  pushCommonButtonsStyle();

    if(ImGui::SmallButton(ICON_KI_PENCIL"##GeometryChangeName"))
    {
      ImGui::OpenPopup("Enter a new name");
      strcpy(newName, geometryName);
    }

  popCommonButtonsStyle();


  if(textInputPopup("Enter a new name", "Enter a new name", newName, 128) == TRUE)
  {
    geometrySetName(geometry, newName);
  }

  pushCommonButtonsStyle();
    ImGui::SameLine();       
    ImGui::SmallButton(ICON_KI_GRID"##GeometryChoose");
    
    ImGui::SameLine();
    ImGui::SmallButton(ICON_KI_COG"##GeometryEdit");
    
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, (float4)ImColor(160, 0, 0, 255));
      ImGui::SmallButton(ICON_KI_TRASH"##GeometryRemove");
    ImGui::PopStyleColor();
    
  if(treeOpen)
  {
    ImGui::PushStyleColor(ImGuiCol_Text, (float4)ImColor(127, 127, 160, 255));

      if(geometryIsLeaf(geometry) == TRUE && geometryHasSDF(geometry) == FALSE)
      {
        ImGui::SmallButton("[New SDF]");
        ImGui::SameLine();
      }

      ImGui::SmallButton("[New IDF]");
      ImGui::SameLine();
      ImGui::SmallButton("[New ODF]");
      ImGui::SameLine();
      ImGui::SmallButton("[New child]");
      
    ImGui::PopStyleColor();

    
    
    if(data->listGeometrySDF)
    {
      ScriptFunction* sdf = geometryGetSDF(geometry);

      // TODO: Show meta info (e.g "Not a leaf" in case it doesn't have an sdf and has the geometry has a
      // a parent, otherwise show "attach new sdf")
      if(sdf != nullptr)
      {
        ImGui::Text("-- [SDF] '%s'", scriptFunctionGetName(sdf).c_str());
        ImGui::SameLine();
        ImGui::SmallButton(ICON_KI_GRID);

        ImGui::SameLine();
        ImGui::SmallButton(ICON_KI_GRID);

        ImGui::SameLine();        
        ImGui::SmallButton(ICON_KI_GRID);        
      }

    }

    if(data->listGeometryIDF)
    {
      const std::vector<ScriptFunction*> idfs = geometryGetIDFs(geometry);

      for(ScriptFunction* idf: idfs)
      {
        ImGui::Text(" -- [IDF] '%s'", scriptFunctionGetName(idf).c_str());
      }


    }

    ImGui::TreePop();
  }

  popCommonButtonsStyle();

  ImGui::PopID();
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
      sceneHierarchyDrawGeometryList(window, geometry, data);
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
