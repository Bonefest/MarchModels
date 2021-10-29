#include <memory_manager.h>

#include "editor.h"
#include "ui_utils.h"
#include "ui_styles.h"
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

static Geometry* createNewGeometry()
{
  ScriptFunction* sphereSDF;
  assert(createScriptFunction(SCRIPT_FUNCTION_TYPE_SDF, "sphereSDF", &sphereSDF));
  scriptFunctionSetArgValue(sphereSDF, "radius", 1.0);

  Geometry* newGeometry;
  assert(createGeometry("sphere", &newGeometry));
  geometrySetSDF(newGeometry, sphereSDF);

  return newGeometry;
}

static void sceneHierarchyDrawGeometryData(Window* window, Geometry* geometry, SceneHierarchyData* data)
{
  const uint32 maxNameSize = 128;
  static char newName[maxNameSize];
  const char* geometryName = geometryGetName(geometry).c_str();
  ImGuiStyle& style = ImGui::GetStyle();  
  
  ImGui::PushID(geometry);

  bool treeOpen = ImGui::TreeNode(geometryName);
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
      ImGui::SmallButton(ICON_KI_TRASH"##GeometryRemove");
    ImGui::PopStyleColor();

    // Geometry content -------------------------------------------------------
    if(treeOpen)
    {

      // Creation buttons -----------------------------------------------------
      ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);

        if(geometryIsLeaf(geometry) == TRUE && geometryHasSDF(geometry) == FALSE)
        {
          if(ImGui::SmallButton("[New SDF]"))
          {
            ScriptFunction* newSphereSDF;
            createScriptFunction(SCRIPT_FUNCTION_TYPE_SDF, "sphereSDF", &newSphereSDF);
            geometrySetSDF(geometry, newSphereSDF);
          }
          ImGui::SameLine();
        }

        if(ImGui::SmallButton("[New IDF]"))
        {
          ScriptFunction* newEmptyIDF;
          createScriptFunction(SCRIPT_FUNCTION_TYPE_IDF, "emptyIDF", &newEmptyIDF);
          geometryAddIDF(geometry, newEmptyIDF);
        }
        
        ImGui::SameLine();
        if(ImGui::SmallButton("[New ODF]"))
        {
          ScriptFunction* newEmptyODF;
          createScriptFunction(SCRIPT_FUNCTION_TYPE_ODF, "emptyODF", &newEmptyODF);
          geometryAddODF(geometry, newEmptyODF);
        }
        
        ImGui::SameLine();
        if(ImGui::SmallButton("[New child]"))
        {
          geometryAddChild(geometry, createNewGeometry());
        }

      ImGui::PopStyleColor();

      // SDF attachment -------------------------------------------------------
      if(data->listGeometrySDF)
      {
        ScriptFunction* sdf = geometryGetSDF(geometry);

        if(sdf != nullptr)
        {
          ImGui::Text("[SDF] '%s'", scriptFunctionGetName(sdf).c_str());
          
          ImGui::SameLine();
          ImGui::SmallButton(ICON_KI_LIST);

          ImGui::SameLine();
          ImGui::SmallButton(ICON_KI_COG);

          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Text, (float4)DeleteClr);
            if(ImGui::SmallButton(ICON_KI_TRASH))
            {
              destroyScriptFunction(sdf);
              geometrySetSDF(geometry, nullptr);
            }
          ImGui::PopStyleColor();
        }

      }

      // IDF attachments ------------------------------------------------------
      if(data->listGeometryIDF)
      {
        std::vector<ScriptFunction*>& idfs = geometryGetIDFs(geometry);

        for(auto idfIt = idfs.begin(); idfIt != idfs.end();)
        {
          bool8 erased = FALSE;
          
          ImGui::PushID(*idfIt);
            ImGui::Text("[IDF] '%s'", scriptFunctionGetName(*idfIt).c_str());
            ImGui::SameLine();
            
            ImGui::SmallButton(ICON_KI_LIST);
            ImGui::SameLine();

            ImGui::SmallButton(ICON_KI_COG);
            ImGui::SameLine();
          
            ImGui::PushStyleColor(ImGuiCol_Text, (float4)DeleteClr);
              if(ImGui::SmallButton(ICON_KI_TRASH))
              {
                destroyScriptFunction(*idfIt);
                idfIt = idfs.erase(idfIt);
                erased = TRUE;
              }
            ImGui::PopStyleColor();
            
          ImGui::PopID();

          if(erased == FALSE)
          {
            idfIt++;
          }
        }

      }

      // ODF attachments ------------------------------------------------------
      if(data->listGeometryODF)
      {
        std::vector<ScriptFunction*>& odfs = geometryGetODFs(geometry);

        for(auto odfIt = odfs.begin(); odfIt != odfs.end();)
        {
          bool8 erased = FALSE;
          
          ImGui::PushID(*odfIt);
            ImGui::Text("[ODF] '%s'", scriptFunctionGetName(*odfIt).c_str());
            ImGui::SameLine();
            
            ImGui::SmallButton(ICON_KI_LIST);
            ImGui::SameLine();

            ImGui::SmallButton(ICON_KI_COG);
            ImGui::SameLine();
          
            ImGui::PushStyleColor(ImGuiCol_Text, (float4)DeleteClr);
              if(ImGui::SmallButton(ICON_KI_TRASH))
              {
                destroyScriptFunction(*odfIt);
                odfIt = odfs.erase(odfIt);
                erased = TRUE;
              }
            ImGui::PopStyleColor();
            
          ImGui::PopID();

          if(erased == FALSE)
          {
            odfIt++;
          }
        }

      }

      
      // Children geometry ----------------------------------------------------
      std::vector<Geometry*> children = geometryGetChildren(geometry);
      for(Geometry* child: children)
      {
        sceneHierarchyDrawGeometryData(window, child, data);
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
  
  if(data->listGeometry)
  {
    const std::vector<Geometry*> geometryArray = sceneGetGeometry(currentScene);
    for(Geometry* geometry: geometryArray)
    {
      sceneHierarchyDrawGeometryData(window, geometry, data);
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
