#include <memory_manager.h>

#include "sdf_editor_view.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

const char* viewName = "SDF Editor";
const char* codeEditorWidgetName = "Code editor";
const char* sdfPreview1WidgetName = "Preview 1";
const char* sdfPreview2WidgetName = "Preview 2";
const char* consoleWidgetName = "Console";

struct SDFEditorData
{
  
};

/**
 * SDF Editor will have approximately the next layout:
 *        +----------+
 *        | Code  |P1|
 *        |       |__|
 *        +-c-n-l-|P2|
 *        |__o_s__|__|
 */
static void updateLayout(uint2 screenSize)
{
  ImGuiID viewNodeID = ImGui::GetID(viewName);

  ImGui::DockBuilderRemoveNode(viewNodeID);
  ImGui::DockBuilderAddNode(viewNodeID, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(viewNodeID, ImVec2(screenSize.x, screenSize.y));

  ImGuiID leftNodeID, rightNodeID;
  ImGui::DockBuilderSplitNode(viewNodeID, ImGuiDir_Left, 0.75f, &leftNodeID, &rightNodeID);

  ImGuiID codeEditorNodeID, consoleNodeID;
  ImGui::DockBuilderSplitNode(leftNodeID, ImGuiDir_Up, 0.75f, &codeEditorNodeID, &consoleNodeID);

  ImGuiID preview1NodeID, preview2NodeID;
  ImGui::DockBuilderSplitNode(rightNodeID, ImGuiDir_Up, 0.5f, &preview1NodeID, &preview2NodeID);

  ImGui::DockBuilderDockWindow(codeEditorWidgetName, codeEditorNodeID);
  ImGui::DockBuilderDockWindow(consoleWidgetName, consoleNodeID);
  ImGui::DockBuilderDockWindow(sdfPreview1WidgetName, preview1NodeID);
  ImGui::DockBuilderDockWindow(sdfPreview2WidgetName, preview2NodeID);

  ImGui::DockBuilderFinish(viewNodeID);
}

static bool8 initializeSDFEditor()
{
  // create two scene widgets, setup one camera for them.
  // first widget will be delayed with better quality
  // second widget will be real-time with fast debug quality
  // create widget for text editing
  // create widget for console output
  return TRUE;
}

static void shutdownSDFEditor()
{

}

static void onLoadSDFEditor()
{

}

static void onUnloadSDFEditor()
{

}

static void onResizeSDFEditor(uint2 newSize)
{

}

static void updateSDFEditor(float64 delta)
{

}

static void drawSDFEditor(float64 delta)
{
  ImGui::Text("Abc");

  ImGui::Begin(codeEditorWidgetName);

  ImGui::Text("Cdef");
  
  ImGui::End();
}

static void processInputSDFEditor(const EventData& eventData, void* sender)
{

}

bool8 createSDFEditorView(View** outView)
{
  *outView = engineAllocObject<View>(MEMORY_TYPE_GENERAL);
  View* view = *outView;
  view->initialize = initializeSDFEditor;
  view->shutdown = shutdownSDFEditor;
  view->onLoad = onLoadSDFEditor;
  view->onUnload = onUnloadSDFEditor;
  view->update = updateSDFEditor;
  view->draw = drawSDFEditor;
  view->processInput = processInputSDFEditor;
  view->name = "SDF Editor";

  return TRUE;
}
