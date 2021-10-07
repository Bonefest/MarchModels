#include "main_view.h"

#include <memory_manager.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <film.h>
#include <camera.h>
#include <debug_ray_integrator.h>
#include <debug_image_integrator.h>
#include <samplers/center_sampler.h>

#include "widgets/text_edit_widget.h"
#include "widgets/image_integrator_display_widget.h"

#include "main_view.h"

struct MainViewData
{
  Widget* codeEditorWidget;
};

static MainViewData viewData;

const char* codeWindowName = "Code##MainView";
const char* consoleWindowName = "Console##MainView";
const char* assetsWindowName = "Assets##MainView";
const char* previewWindowName = "Preview##MainView";
const char* contextWindowName = "Context##MainView";
const char* sceneHierarchyWindowName = "Scene hierarchy##MainView";
const char* actionWindowName = "Actions##MainView";

static bool8 mainViewInitialize(View* view)
{
  assert(createTextEditWidget(codeWindowName, &viewData.codeEditorWidget));
  
  return TRUE;
}

static void mainViewShutdown(View* view)
{
  freeWidget(viewData.codeEditorWidget);
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

  ImGuiID lgroupNodeID, rgroupNodeID;
  ImGui::DockBuilderSplitNode(viewNodeID, ImGuiDir_Left, 0.25f, &lgroupNodeID, &rgroupNodeID);

  // Finding ids of nodes of the first column (Code, console, assets)
  ImGuiID codeNodeID, consoleNodeID, assetsNodeID;
  {
    ImGuiID ugroupNodeID;
    ImGui::DockBuilderSplitNode(lgroupNodeID, ImGuiDir_Up, 0.5f, &ugroupNodeID, &assetsNodeID);

    { 
      ImGui::DockBuilderSplitNode(ugroupNodeID, ImGuiDir_Up, 0.7f, &codeNodeID, &consoleNodeID);       
    }
      
  }

  ImGui::DockBuilderSplitNode(rgroupNodeID, ImGuiDir_Left, 0.66f, &lgroupNodeID, &rgroupNodeID);

  // Finding ids of nodes of the second column (Preview, Context)
  ImGuiID previewNodeID, contextNodeID;
  {
    ImGui::DockBuilderSplitNode(lgroupNodeID, ImGuiDir_Up, 0.5f, &previewNodeID, &contextNodeID);
  }

  // Finding ids of nodes of the third column (Hierarchy, Action)
  ImGuiID sceneHierarchyNodeID, actionNodeID;
  {
    ImGui::DockBuilderSplitNode(rgroupNodeID, ImGuiDir_Up, 0.5f, &sceneHierarchyNodeID, &actionNodeID);
  }
  
  ImGui::DockBuilderDockWindow(codeWindowName, codeNodeID);
  ImGui::DockBuilderDockWindow(consoleWindowName, consoleNodeID);
  ImGui::DockBuilderDockWindow(assetsWindowName, assetsNodeID);
  ImGui::DockBuilderDockWindow(previewWindowName, previewNodeID);
  ImGui::DockBuilderDockWindow(contextWindowName, contextNodeID);  
  ImGui::DockBuilderDockWindow(sceneHierarchyWindowName, sceneHierarchyNodeID);  
  ImGui::DockBuilderDockWindow(actionWindowName, actionNodeID);
  
  ImGui::DockBuilderFinish(viewNodeID);  
}

static void mainViewUpdate(View* view, float64 delta)
{
  updateWidget(viewData.codeEditorWidget, view, delta);
}

static void mainViewDraw(View* view, ImVec2 viewOffset, ImVec2 viewSize, float64 delta)
{
  drawWidget(viewData.codeEditorWidget, view, delta);
  
  ImGui::Begin(consoleWindowName);
  ImGui::Button(ICON_KI_INFO " Info");
  ImGui::End();
  
  ImGui::Begin(assetsWindowName);
  ImGui::End();
  
  ImGui::Begin(previewWindowName);
  ImGui::End();
  
  ImGui::Begin(contextWindowName);
  ImGui::End();

  ImGui::Begin(sceneHierarchyWindowName);
  ImGui::End();
  
  ImGui::Begin(actionWindowName);
  ImGui::End();  
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
