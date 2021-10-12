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

#include "editor.h"
#include "windows/view_window.h"
#include "windows/console_window.h"

using std::vector;
using std::string;
using std::unordered_map;

struct EditorData
{
  Scene* currentScene;
  Camera* camera;

  Window* toolbarWindow = NULL;
  Window* viewWindow;
  Window* sceneHierarchyWindow = NULL;  
  Window* consoleWindow;

  vector<Window*> openedWindows;
};

static EditorData editorData;

const char* toolbarWindowName = "Toolbar##EditorWindow";
const char* viewWindowName = "View##EditorWindow";
const char* sceneHierarchyWindowName = "Scene hierarchy##EditorWindow";
const char* consoleWindowName = "Console##EditorWindow";

// ----------------------------------------------------------------------------
// Initialization-related functions
// ----------------------------------------------------------------------------
bool8 initEditor(Application* app)
{
  return TRUE;
}

void shutdownEditor(Application* app)
{

}

// ----------------------------------------------------------------------------
// General API functions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Internal logic
// ----------------------------------------------------------------------------

void updateEditor(Application* app, float64 delta)
{
  
}

static void prepareDockingLayout(float2 screenSize)
{
  const char* dockingRootName = "DockingRoot";
  ImGuiID dockingRootID = ImGui::GetID(dockingRootName);

  // Prepare docked nodes only if we didn't it before (no node is attach to a dock builder)
  if(ImGui::DockBuilderGetNode(dockingRootID) == nullptr)
  {
    ImGui::DockBuilderAddNode(dockingRootID, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockingRootID, ImVec2(screenSize.x, screenSize.y));

    ImGuiID toolbarNodeID, dgroupNodeID;
    ImGui::DockBuilderSplitNode(dockingRootID, ImGuiDir_Up, 0.1f, &toolbarNodeID, &dgroupNodeID);

    ImGuiID lgroupNodeID, sceneHierarchyNodeID;
    ImGui::DockBuilderSplitNode(dgroupNodeID, ImGuiDir_Left, 0.6f, &lgroupNodeID, &sceneHierarchyNodeID);
  
    ImGuiID viewNodeID, consoleNodeID;
    ImGui::DockBuilderSplitNode(lgroupNodeID, ImGuiDir_Down, 0.33f, &consoleNodeID, &viewNodeID);

    ImGui::DockBuilderDockWindow(toolbarWindowName, toolbarNodeID);
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

  ImGui::Begin(dockingRootName, nullptr, dockWindowFlags);
    ImGui::DockSpace(dockingRootID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);  
  ImGui::End();
}

void drawEditor(Application* app, float64 delta)
{
  uint32 screenWidth = applicationGetScreenWidth(), screenHeight = applicationGetScreenHeight();
  prepareDockingLayout(float2(screenWidth, screenHeight));

  ImGui::Begin(consoleWindowName);
  ImGui::End();
  
  ImGui::Begin(toolbarWindowName);
  ImGui::Button(ICON_KI_COMPUTER" Scenes");
  ImGui::End();
  
  ImGui::Begin(viewWindowName);
  ImGui::End();

  ImGui::Begin(sceneHierarchyWindowName);
  ImGui::End();
  
  ImGui::ShowDemoWindow();

  for(Window* window: editorData.openedWindows)
  {
    drawWindow(window, delta);
  }
  
}

void processInputEditor(Application* app, const EventData& eventData, void* sender)
{
  
}
