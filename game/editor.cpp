#if defined(ENABLE_EDITOR_GAME)

#include <vector>
#include <string>
#include <unordered_map>

#include <imgui/imgui.h>
#include <linalg/linalg.h>
#include <imgui/imgui_internal.h>

#include <film.h>
#include <camera.h>
#include <sampler.h>
#include <logging.h>
#include <dfunction.h>
#include <application.h>
#include <memory_manager.h>
#include <game_framework.h>

#include "editor.h"
#include "views/sdf_editor_view.h"

using std::string;
using std::unordered_map;

struct EditorInternalData
{
  unordered_map<string, View*> viewsMap;
  View* currentView = nullptr;
};

static EditorInternalData editorInternalData;

static bool8 extractSetupConfig(Application* app,
                                uint32* outScreenWidth,
                                uint32* outScreenHeight,
                                const char** outName);

static bool8 initialize(Application* app);

static void shutdown(Application* app);
static void update(Application* app, float64 delta);
static void draw(Application* app, float64 delta);
static void processInput(Application* app, const EventData& eventData, void* sender);

// ----------------------------------------------------------------------------
// Initialization-related functions
// ----------------------------------------------------------------------------
bool8 initializeGameFramework(GameFramework* outFramework)
{
  outFramework->extractSetupConfig = extractSetupConfig;
  outFramework->initialize = initialize;
  outFramework->shutdown = shutdown;
  outFramework->update = update;
  outFramework->draw = draw;
  outFramework->processInput = processInput;

  return TRUE;
}

bool8 extractSetupConfig(Application* app,
                         uint32* outScreenWidth,
                         uint32* outScreenHeight,
                         const char** outName)
{
  *outScreenWidth = 1280;
  *outScreenHeight = 720;
  *outName = "Editor";

  return TRUE;
}


static bool8 initialize(Application* app)
{
  uint32 screenWidth = applicationGetScreenWidth(), screenHeight = applicationGetScreenHeight();
  
  View* sdfEditorView = nullptr;
  assert(createSDFEditorView(uint2(screenWidth, screenHeight), &sdfEditorView));
  editorInternalData.viewsMap[viewGetName(sdfEditorView)] = sdfEditorView;

  for(auto viewPair: editorInternalData.viewsMap)
  {
    assert(initializeView(viewPair.second));
  }

  editorSetView(viewGetName(sdfEditorView));
  
  return TRUE;
}

static void shutdown(Application* app)
{
  for(auto viewPair: editorInternalData.viewsMap)
  {
    destroyView(viewPair.second);
  }

  editorInternalData.viewsMap.clear();
}

// ----------------------------------------------------------------------------
// General API functions
// ----------------------------------------------------------------------------

bool8 editorSetView(const std::string& viewName)
{
  auto viewIt = editorInternalData.viewsMap.find(viewName);
  if(viewIt == editorInternalData.viewsMap.end())
  {
    LOG_ERROR("View \"%s\" is not found!", viewName.c_str());
    return FALSE;
  }

  if(editorInternalData.currentView != nullptr)
  {
    viewOnUnload(editorInternalData.currentView);
  }
  
  editorInternalData.currentView = viewIt->second;
  viewOnLoad(editorInternalData.currentView);
  
  return TRUE;
}

View* editorGetCurrentView()
{
  return editorInternalData.currentView;
}

// ----------------------------------------------------------------------------
// Internal logic
// ----------------------------------------------------------------------------

static void update(Application* app, float64 delta)
{
  updateView(editorInternalData.currentView, delta);
}

static void draw(Application* app, float64 delta)
{
  uint32 screenWidth = applicationGetScreenWidth(), screenHeight = applicationGetScreenHeight();    

  ImGui::BeginMainMenuBar();
  if(ImGui::BeginMenu("New"))
  {
    if(ImGui::MenuItem("Scene editor"))
    {
      
    }

    else if(ImGui::MenuItem("SDF"))
    {

    }

    else if(ImGui::MenuItem("IDF"))
    {
      
    }

    else if(ImGui::MenuItem("ODF"))
    {

    }

    else if(ImGui::MenuItem("Shape"))
    {

    }

    else if(ImGui::MenuItem("Material"))
    {

    }
    
    ImGui::EndMenu();
  }
  ImGui::EndMainMenuBar();

  drawView(editorInternalData.currentView, delta);
}

static void processInput(Application* app, const EventData& eventData, void* sender)
{
  processInputView(editorInternalData.currentView, eventData, sender);
}

// Editor GUI consists of many windows:
//  - Scene window (Has mode: Debug/Simple/Beauty, Has type: Real-time/Generated)
//  - Code preview window
//  - Scene tree window
//  - Settings window
//  - Console window
//
// Windows are combined in Views. Views are collections of windows.
// View is a state.
// Each window is implemented as a distinct sub-system

#endif
