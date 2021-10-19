#include <vector>
#include <string>
#include <unordered_map>

#include <imgui/imgui.h>
#include <linalg/linalg.h>
#include <imgui/imgui_internal.h>

#include <film.h>
#include <camera.h>
#include <logging.h>
#include <application.h>
#include <memory_manager.h>
#include <game_framework.h>
#include <script_function.h>
#include <debug_ray_integrator.h>
#include <samplers/center_sampler.h>

#include "editor.h"
#include "windows/view_window.h"
#include "windows/console_window.h"
#include "windows/window_manager.h"

using std::vector;
using std::string;
using std::unordered_map;

struct EditorData
{
  Scene* currentScene = nullptr;
  Camera* camera;

  Window* viewWindow;
  Window* sceneHierarchyWindow = nullptr;
  Window* consoleWindow;

  WindowManager* windowManager;
};

static EditorData editorData;

const char* viewWindowName = "View##EditorWindow";
const char* sceneHierarchyWindowName = "Scene hierarchy##EditorWindow";
const char* consoleWindowName = "Console##EditorWindow";

// ----------------------------------------------------------------------------
// Initialization-related functions
// ----------------------------------------------------------------------------

bool8 initEditor(Application* app)
{
  // TEMP
  vector<string> params = {"radius"};
  assert(declareScriptFunction(SCRIPT_FUNCTION_TYPE_SDF,
                               "sphereSDF",
                               "return float3.length(args[\"p\"]) - args[\"radius\"]",
                               params));

  ScriptFunction* sdf = nullptr;
  assert(createScriptFunction(SCRIPT_FUNCTION_TYPE_SDF, "sphereSDF", &sdf));
  scriptFunctionSetArgValue(sdf, "radius", 3.0);
  
  Geometry* geometry = nullptr;
  assert(createGeometry("sphere", &geometry));
  geometrySetSDF(geometry, sdf);
  geometrySetPosition(geometry, float3(0.0f, 0.0f, 10.0f));
  
  assert(createScene(&editorData.currentScene));
  sceneAddGeometry(editorData.currentScene, geometry);
  // END TEMP

  assert(createWindowManager(&editorData.windowManager));
  
  assert(createConsoleWindow(consoleWindowName, &editorData.consoleWindow));
  windowManagerAddWindow(editorData.windowManager, editorData.consoleWindow);

  Sampler* centerSampler = nullptr;
  assert(createCenterSampler(uint2(0, 0), &centerSampler));
  
  RayIntegrator* debugRayIntegrator = nullptr;
  assert(createDebugRayIntegrator(DEBUG_RAY_INTEGRATOR_MODE_ONE_COLOR, &debugRayIntegrator));

  Camera* camera = nullptr;
  assert(createPerspectiveCamera(1.0f, toRad(45.0f), 0.01f, 100.0f, &camera));
  
  assert(createViewWindow(viewWindowName, centerSampler, debugRayIntegrator, camera, &editorData.viewWindow));
  windowManagerAddWindow(editorData.windowManager, editorData.viewWindow);
  
  return TRUE;
}

void shutdownEditor(Application* app)
{
  destroyWindowManager(editorData.windowManager);
}

// ----------------------------------------------------------------------------
// General API functions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Internal logic
// ----------------------------------------------------------------------------

void updateEditor(Application* app, float64 delta)
{
  windowManagerUpdate(editorData.windowManager, delta);
}

static void prepareDockingLayout(float2 screenSize, float2 screenOffset)
{
  const char* dockingRootName = "DockingRoot";
  ImGuiID dockingRootID = ImGui::GetID(dockingRootName);

  // Prepare docked nodes only if we didn't it before (no node is attached to a dock builder)
  if(ImGui::DockBuilderGetNode(dockingRootID) == nullptr)
  {
    ImGui::DockBuilderRemoveNode(dockingRootID);
    ImGui::DockBuilderAddNode(dockingRootID, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockingRootID, screenSize);

    ImGuiID lgroupNodeID, sceneHierarchyNodeID;
    ImGui::DockBuilderSplitNode(dockingRootID, ImGuiDir_Left, 0.6f, &lgroupNodeID, &sceneHierarchyNodeID);
  
    ImGuiID viewNodeID, consoleNodeID;
    ImGui::DockBuilderSplitNode(lgroupNodeID, ImGuiDir_Down, 0.25f, &consoleNodeID, &viewNodeID);

    ImGui::DockBuilderDockWindow(viewWindowName, viewNodeID);
    ImGui::DockBuilderDockWindow(sceneHierarchyWindowName, sceneHierarchyNodeID);
    ImGui::DockBuilderDockWindow(consoleWindowName, consoleNodeID);
  
    ImGui::DockBuilderFinish(dockingRootID);      
  }
  
  static const ImGuiWindowFlags dockWindowFlags =
    ImGuiWindowFlags_NoTitleBar |
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoScrollbar |
    ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoBackground |
    ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_NoDocking |
    ImGuiWindowFlags_NoNavFocus;

  ImGui::SetNextWindowPos(screenOffset);
  ImGui::SetNextWindowSize(screenSize);
  
  ImGui::Begin(dockingRootName, nullptr, dockWindowFlags);
    ImGui::DockSpace(dockingRootID, float2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);  
  ImGui::End();
}

void drawMenu(float2& outMenuSize)
{
  ImGui::BeginMainMenuBar();
    if(ImGui::BeginMenu(ICON_KI_COMPUTER" Scene"))
    {
      if(ImGui::MenuItem("New"))
      {
        
      }

      if(ImGui::MenuItem("Open"))
      {
        
      }

      if(ImGui::MenuItem("Save"))
      {

      }
      
      ImGui::Separator();

      if(ImGui::MenuItem("Settings"))
      {

      }
      
      ImGui::EndMenu();
    }

  outMenuSize = ImGui::GetWindowSize();
    
  ImGui::EndMainMenuBar();
}

void drawEditor(Application* app, float64 delta)
{
  uint32 screenWidth = applicationGetScreenWidth(), screenHeight = applicationGetScreenHeight();

  float2 menuSize;
  drawMenu(menuSize);
  
  prepareDockingLayout(float2(screenWidth, screenHeight - menuSize.y),
                       float2(0.0f, menuSize.y));

  ImGui::Begin(sceneHierarchyWindowName);
  ImGui::End();
  
  ImGui::ShowDemoWindow();

  windowManagerDraw(editorData.windowManager, delta);
}

void processInputEditor(Application* app, const EventData& eventData, void* sender)
{
  windowManagerProcessInput(editorData.windowManager, eventData, sender);
}

void editorSetScene(Scene* scene)
{
  // TODO: Notify all that scene has changed
  editorData.currentScene = scene;
}

Scene* editorGetCurrentScene()
{
  return editorData.currentScene;
}
