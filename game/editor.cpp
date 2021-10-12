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

const char* viewWindowName = "View##EditorWindow";
const char* sceneHierarchyWindowName = "Scene hierarchy##EditorWindow";
const char* consoleWindowName = "Console##EditorWindow";

// ----------------------------------------------------------------------------
// Initialization-related functions
// ----------------------------------------------------------------------------

bool8 initEditor(Application* app)
{
  assert(createConsoleWindow(consoleWindowName, &editorData.consoleWindow));
  editorData.openedWindows.push_back(editorData.consoleWindow);

  for(Window* window: editorData.openedWindows)
  {
    initWindow(window);
  }
  
  return TRUE;
}

void shutdownEditor(Application* app)
{
  for(Window* window: editorData.openedWindows)
  {
    freeWindow(window);
  }
}

// ----------------------------------------------------------------------------
// General API functions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Internal logic
// ----------------------------------------------------------------------------

void updateEditor(Application* app, float64 delta)
{
  for(Window* window: editorData.openedWindows)
  {
    updateWindow(window, delta);
  }
}

static void prepareDockingLayout(ImVec2 screenSize, ImVec2 screenOffset)
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
    ImGui::DockSpace(dockingRootID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);  
  ImGui::End();
}

void drawMenu(ImVec2& outMenuSize)
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

  ImVec2 menuSize;
  drawMenu(menuSize);
  
  prepareDockingLayout(ImVec2(screenWidth, screenHeight - menuSize.y),
                       ImVec2(0.0f, menuSize.y));
  
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
  for(Window* window: editorData.openedWindows)
  {
    processInputWindow(window, eventData, sender);
  }
}
