#include <memory_manager.h>

#include "sdf_editor_view.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

const char* viewName = "SDF Editor";
const char* codeEditorWidgetName = "Code editor";
const char* sdfPreview1WidgetName = "Preview 1";
const char* sdfPreview2WidgetName = "Preview 2";
const char* consoleWidgetName = "Console";

/**
 * SDF Editor will have approximately the next layout:
 *        +----------+
 *        | Code  |P1|
 *        |       |__|
 *        +-c-n-l-|P2|
 *        |__o_s__|__|
 */
static void updateSDFEditorLayout(View* view, ImVec2 viewSize)
{
  ImGuiID viewNodeID = viewGetMainNodeID(view);

  ImGui::DockBuilderRemoveNode(viewNodeID);
  ImGui::DockBuilderAddNode(viewNodeID, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(viewNodeID, viewSize);

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

static bool8 initializeSDFEditor(View* view)
{
  // create two scene widgets, setup one camera for them.
  // first widget will be delayed with better quality
  // second widget will be real-time with fast debug quality
  // create widget for text editing
  // create widget for console output
  return TRUE;
}

static void shutdownSDFEditor(View* view)
{

}

static void onLoadSDFEditor(View* view)
{

}

static void onUnloadSDFEditor(View* view)
{

}

static void updateSDFEditor(View* view, float64 delta)
{

}

static void drawSDFEditor(View* view, ImVec2 viewOffset, ImVec2 viewSize, float64 delta)
{
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin(codeEditorWidgetName);
  ImGui::PopStyleVar();
  ImGui::Text("Cdef");
  ImGui::End();
}

static void processInputSDFEditor(View* view, const EventData& eventData, void* sender)
{

}

bool8 createSDFEditorView(View** outView)
{
  ViewInterface interface = {};
  interface.initialize = initializeSDFEditor;
  interface.shutdown = shutdownSDFEditor;
  interface.onLoad = onLoadSDFEditor;
  interface.onUnload = onUnloadSDFEditor;
  interface.updateLayout = updateSDFEditorLayout;
  interface.update = updateSDFEditor;
  interface.draw = drawSDFEditor;
  interface.processInput = processInputSDFEditor;

  return createView("SDF Editor", interface, outView);
}
