#include "main_view.h"

#include <memory_manager.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <film.h>
#include <camera.h>
#include <debug_ray_integrator.h>
#include <debug_image_integrator.h>
#include <samplers/center_sampler.h>

#include "widgets/console_widget.h"
#include "widgets/image_integrator_display_widget.h"

#include "main_view.h"

struct MainViewData
{
  Widget* consoleWidget;
};

static MainViewData viewData;

//const char* codeWindowName = "Code##MainView";
//const char* assetsWindowName = "Assets##MainView";
// const char* contextWindowName = "Context##MainView";
//const char* actionWindowName = "Actions##MainView";

const char* toolbarWindowName = "Toolbar##MainView";
const char* previewWindowName = "Preview##MainView";
const char* sceneHierarchyWindowName = "Scene hierarchy##MainView";
const char* consoleWindowName = "Console##MainView";

static bool8 mainViewInitialize(View* view)
{
  assert(createConsoleWidget(consoleWindowName, &viewData.consoleWidget));
  assert(initializeWidget(viewData.consoleWidget));
  
  return TRUE;
}

static void mainViewShutdown(View* view)
{
  freeWidget(viewData.consoleWidget);
}

static void mainViewOnLoad(View* view)
{

}

static void mainViewOnUnload(View* view)
{

}

static void mainViewUpdateLayout(View* view, ImVec2 viewSize)
{
  ImGuiID viewNodeID = viewGetMainNodeID(view);

  ImGui::DockBuilderRemoveNode(viewNodeID);
  ImGui::DockBuilderAddNode(viewNodeID, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(viewNodeID, viewSize);

  ImGuiID toolbarNodeID, dgroupNodeID;
  ImGui::DockBuilderSplitNode(viewNodeID, ImGuiDir_Up, 0.1f, &toolbarNodeID, &dgroupNodeID);

  ImGuiID lgroupNodeID, sceneHierarchyNodeID;
  ImGui::DockBuilderSplitNode(dgroupNodeID, ImGuiDir_Left, 0.6f, &lgroupNodeID, &sceneHierarchyNodeID);
  
  ImGuiID previewNodeID, consoleNodeID;
  ImGui::DockBuilderSplitNode(lgroupNodeID, ImGuiDir_Down, 0.33f, &consoleNodeID, &previewNodeID);

  ImGui::DockBuilderDockWindow(toolbarWindowName, toolbarNodeID);
  ImGui::DockBuilderDockWindow(previewWindowName, previewNodeID);
  ImGui::DockBuilderDockWindow(sceneHierarchyWindowName, sceneHierarchyNodeID);
  ImGui::DockBuilderDockWindow(consoleWindowName, consoleNodeID);
  
  ImGui::DockBuilderFinish(viewNodeID);  
}

static void mainViewUpdate(View* view, float64 delta)
{
  updateWidget(viewData.consoleWidget, view, delta);
}

static void mainViewDraw(View* view, ImVec2 viewOffset, ImVec2 viewSize, float64 delta)
{
  drawWidget(viewData.consoleWidget, view, delta);

  ImGui::Begin(toolbarWindowName);
  ImGui::End();
  
  ImGui::Begin(previewWindowName);
  ImGui::End();

  ImGui::Begin(sceneHierarchyWindowName);
  ImGui::End();
  
  ImGui::ShowDemoWindow();
}

static void mainViewProcessInput(View* view, const EventData& eventData, void* sender)
{

}

bool8 createMainView(View** outView)
{
  ViewInterface interface = {};
  interface.initialize = mainViewInitialize;
  interface.shutdown = mainViewShutdown;
  interface.onLoad = mainViewOnLoad;
  interface.onUnload = mainViewOnUnload;
  interface.updateLayout = mainViewUpdateLayout;
  interface.update = mainViewUpdate;
  interface.draw = mainViewDraw;
  interface.processInput = mainViewProcessInput;
  
  return createView("MainView", interface, outView);
}
