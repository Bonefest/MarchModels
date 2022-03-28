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
#include <assets/assets_manager.h>
#include <assets/script_function.h>
#include <assets/pcf_script_function.h>
#include <ray_integrators/debug_ray_integrator.h>
#include <samplers/center_sampler.h>

#include "editor.h"
#include "windows/view_window.h"
#include "windows/console_window.h"
#include "windows/window_manager.h"
#include "windows/assets_manager_window.h"
#include "windows/scene_hierarchy_window.h"

using std::vector;
using std::string;
using std::unordered_map;

struct EditorData
{
  Scene* currentScene = nullptr;
  Camera* camera;

  WindowPtr viewWindow;
  WindowPtr sceneHierarchyWindow;
  WindowPtr consoleWindow;

  vector<AssetWPtr> selectedGeometry;
};

static EditorData editorData;

const char* viewWindowName = "View##EditorWindow";
const char* sceneHierarchyWindowName = "Scene hierarchy##EditorWindow";
const char* consoleWindowName = "Console##EditorWindow";

// ----------------------------------------------------------------------------
// Initialization-related functions
// ----------------------------------------------------------------------------

static void declareDefaultScriptFunctions()
{
  Asset *sphereSDFPrototype = nullptr,
    *emptyIDFPrototype = nullptr,
    *emptyODFPrototype = nullptr,
    *intersectionPCFPrototype = nullptr,
    *unionPCFPrototype = nullptr,
    *subtractionPCFPrototype = nullptr;

  createScriptFunction(SCRIPT_FUNCTION_TYPE_SDF,
                       "sphereSDF",
                       &sphereSDFPrototype);
  scriptFunctionSetArgValue(sphereSDFPrototype, "radius", 1.0f);
  scriptFunctionSetCode(sphereSDFPrototype, "return length(p) - $radius; ");
    
  createScriptFunction(SCRIPT_FUNCTION_TYPE_IDF,
                       "emptyIDF",
                       &emptyIDFPrototype);  
  scriptFunctionSetCode(emptyIDFPrototype, "return p; ");
  
  createScriptFunction(SCRIPT_FUNCTION_TYPE_ODF,
                       "emptyODF", 
                       &emptyODFPrototype);
  scriptFunctionSetCode(emptyODFPrototype, "return d;");

  createPCF("intersectionPCF", PCF_NATIVE_TYPE_INTERSECTION, &intersectionPCFPrototype);
  scriptFunctionSetCode(intersectionPCFPrototype, "return (d1 > d2 ? float2(d1, 0.0) : float2(d2, 1.0));");

  createPCF("unionPCF", PCF_NATIVE_TYPE_UNION, &unionPCFPrototype);
  scriptFunctionSetCode(unionPCFPrototype, "return (d1 < d2 ? float2(d1, 0.0) : float2(d2, 1.0));");  

  createPCF("subtractionPCF", PCF_NATIVE_TYPE_SUBTRACTION, &subtractionPCFPrototype);
  scriptFunctionSetCode(subtractionPCFPrototype, "return (d1 > -d2 ? float2(d1, 0.0) : float2(-d2, 1.0));");  
  
  assetsManagerAddAsset(AssetPtr(sphereSDFPrototype));
  assetsManagerAddAsset(AssetPtr(emptyIDFPrototype));
  assetsManagerAddAsset(AssetPtr(emptyODFPrototype));
  
  assetsManagerAddAsset(AssetPtr(intersectionPCFPrototype));
  assetsManagerAddAsset(AssetPtr(unionPCFPrototype));
  assetsManagerAddAsset(AssetPtr(subtractionPCFPrototype));    
}

bool8 initEditor(Application* app)
{
  declareDefaultScriptFunctions();

  assert(createScene(&editorData.currentScene));
  assert(initWindowManager());

  Window* consoleWindow = nullptr;
  assert(createConsoleWindow(consoleWindowName, &consoleWindow));
  editorData.consoleWindow = WindowPtr(consoleWindow);
  windowManagerAddWindow(editorData.consoleWindow);

  Sampler* centerSampler = nullptr;
  assert(createCenterSampler(uint2(0, 0), &centerSampler));
  
  RayIntegrator* debugRayIntegrator = nullptr;
  assert(createDebugRayIntegrator(DEBUG_RAY_INTEGRATOR_MODE_ONE_COLOR, &debugRayIntegrator));

  Camera* camera = nullptr;
  assert(createPerspectiveCamera(1.0f, toRad(45.0f), 0.1f, 30.0f, &camera));
  cameraSetPosition(camera, float3(0.0f, 0.0f, -10.0f));

  Window* viewWindow = nullptr;
  assert(createViewWindow(viewWindowName, centerSampler, debugRayIntegrator, camera, &viewWindow));
  editorData.viewWindow = WindowPtr(viewWindow);
  windowManagerAddWindow(editorData.viewWindow);

  Window* sceneHierarchyWindow = nullptr;
  assert(createSceneHierarchyWindow(sceneHierarchyWindowName, &sceneHierarchyWindow));
  editorData.sceneHierarchyWindow = WindowPtr(sceneHierarchyWindow);
  windowManagerAddWindow(editorData.sceneHierarchyWindow);
  
  return TRUE;
}

void shutdownEditor(Application* app)
{
  shutdownWindowManager();
}

// ----------------------------------------------------------------------------
// General API functions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Internal logic
// ----------------------------------------------------------------------------

void editorUpdate(Application* app, float64 delta)
{
  windowManagerUpdate(delta);
  updateScene(editorData.currentScene, delta);
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

    if(ImGui::BeginMenu(ICON_KI_LIST" Assets list"))
    {
      if(windowManagerHasWindow(assetsManagerWindowGetIdentifier()) == FALSE)
      {
        Window* assetsManagerWindow = nullptr;
        assert(createAssetsManagerWindow(&assetsManagerWindow));
        windowManagerAddWindow(WindowPtr(assetsManagerWindow));
      }
      
      ImGui::EndMenu();
    }
    
  outMenuSize = ImGui::GetWindowSize();
    
  ImGui::EndMainMenuBar();
}

void editorDraw(Application* app, float64 delta)
{
  uint32 screenWidth = applicationGetScreenWidth(), screenHeight = applicationGetScreenHeight();

  float2 menuSize;
  drawMenu(menuSize);
  
  prepareDockingLayout(float2(screenWidth, screenHeight - menuSize.y),
                       float2(0.0f, menuSize.y));

  ImGui::ShowDemoWindow();

  windowManagerDraw(delta);
}

void editorProcessInput(Application* app, const EventData& eventData, void* sender)
{
  windowManagerProcessInput(eventData, sender);
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

static bool8 traverseGeometryUnselect(Asset* geometry, void* udata)
{
  geometrySetSelected(geometry, FALSE);
  return FALSE;
}

void editorClearSelectedGeometry()
{
  if(editorData.currentScene != nullptr)
  {
    geometryTraversePostorder(sceneGetGeometryRoot(editorData.currentScene),
                              traverseGeometryUnselect);
  }
}

std::vector<AssetPtr> editorGetSelectedGeometry()
{
  std::vector<AssetPtr> selectedGeometry;
  const std::set<AssetPtr>& geometryList = sceneGetAllChildren(editorData.currentScene);
  for(AssetPtr geometry: geometryList)
  {
    if(geometryIsSelected(geometry))
    {
      selectedGeometry.push_back(geometry);
    }
  }

  return selectedGeometry;
}

void editorClearSelectedLightSources()
{
  if(editorData.currentScene != nullptr)
  {
    std::vector<AssetPtr>& lightSources = sceneGetLightSources(editorData.currentScene);
    for(AssetPtr lightSource: lightSources)
    {
      lightSourceSetSelected(lightSource, FALSE);
    }
  }
}

std::vector<AssetPtr> editorGetSelectedLightSources()
{
  std::vector<AssetPtr> selectedLightSources;
  std::vector<AssetPtr>& lightSources = sceneGetLightSources(editorData.currentScene);
  std::copy_if(lightSources.begin(), lightSources.end(),
               std::back_inserter(selectedLightSources),
               [](AssetPtr lsource) { return lightSourceIsSelected(lsource) == TRUE; });

  return selectedLightSources;
}
