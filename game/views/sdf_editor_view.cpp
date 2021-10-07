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

#include "sdf_editor_view.h"

const char* viewName = "SDF Editor";
const char* codeEditorWidgetName = "Code editor##sdf_editor";
const char* sdfPreview1WidgetName = "Preview 1##sdf_editor";
const char* sdfPreview2WidgetName = "Preview 2##sdf_editor";
const char* consoleWidgetName = "Console##sdf_editor";

const char* v1 = "v1";
const char* v2 = "v2";
const char* v3 = "v3";
const char* v4 = "v4";
const char* v5 = "v5";
const char* v6 = "v6";

struct SDFEditorInternalData
{
  Widget* textEditorWidget;
  Widget* imageIntegratorDisplayWidget;

  ScriptFunction* sphereSDF;
  Geometry* geometry;
  
  Scene* scene;
  Camera* camera;
  Film* film;
  RayIntegrator* rayIntegrator;
  Sampler* sampler;
  ImageIntegrator* imageIntegrator;
};

static SDFEditorInternalData internalData;

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
  vector<string> parameters = {"r"};
  declareScriptFunction(SCRIPT_FUNCTION_TYPE_SDF,
                        "sphereSDF",
                        "return float3.length(args[\"p\"]) - args[\"r\"]",
                        parameters);
  assert(createScriptFunction(SCRIPT_FUNCTION_TYPE_SDF, "sphereSDF", &internalData.sphereSDF));
  scriptFunctionSetArgValue(internalData.sphereSDF, "r", 1.0f);
  
  assert(createGeometry("Test geometry", &internalData.geometry));
  geometrySetSDF(internalData.geometry, internalData.sphereSDF);
  geometrySetPosition(internalData.geometry, float3(0.0f, 0.0f, 5.0f));
  
  assert(createScene(&internalData.scene));
  sceneAddGeometry(internalData.scene, internalData.geometry);
  
  assert(createFilm(uint2(64, 64), &internalData.film));
  assert(createPerspectiveCamera(1.0f, toRad(45.0f), 0.01f, 100.0f, &internalData.camera));
  
  assert(createDebugRayIntegrator(DEBUG_RAY_INTEGRATOR_MODE_ONE_COLOR, &internalData.rayIntegrator));
  assert(createCenterSampler(uint2(64, 64), &internalData.sampler));
  assert(createDebugImageIntegrator(uint2(13, 13), uint2(0,0),
                                    internalData.scene,
                                    internalData.sampler,
                                    internalData.rayIntegrator,
                                    internalData.film,
                                    internalData.camera,
                                    &internalData.imageIntegrator));
  
  assert(createTextEditWidget(codeEditorWidgetName, &internalData.textEditorWidget));
  assert(createImageIntegratorDisplayWidget(sdfPreview1WidgetName,
                                            internalData.imageIntegrator,
                                            10.0f,
                                            &internalData.imageIntegratorDisplayWidget));
  
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
  updateWidget(internalData.textEditorWidget, view, delta);
  updateWidget(internalData.imageIntegratorDisplayWidget, view, delta);  
}

static void drawSDFEditor(View* view, ImVec2 viewOffset, ImVec2 viewSize, float64 delta)
{
  drawWidget(internalData.textEditorWidget, view, delta);
  drawWidget(internalData.imageIntegratorDisplayWidget, view, delta);
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
