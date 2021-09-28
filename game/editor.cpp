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

#include "views/sdf_editor_view.h"

using std::string;
using std::unordered_map;

struct EditorInternalData
{
  unordered_map<string, View*> viewsMap;
  View* currentView;
};

static EditorInternalData editorInternalData;

static const ImGuiWindowFlags viewWindowFlags =
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

static bool8 extractSetupConfig(Application* app,
                                uint32* outScreenWidth,
                                uint32* outScreenHeight,
                                const char** outName);

static bool8 initialize(Application* app);

static void shutdown(Application* app);
static void update(Application* app, float64 delta);
static void draw(Application* app, float64 delta);
static void processInput(Application* app, const EventData& eventData, void* sender);


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
  View* sdfEditorView = nullptr;
  assert(createSDFEditorView(&sdfEditorView));
  editorInternalData.viewsMap[sdfEditorView->name] = sdfEditorView;

  for(auto viewPair: editorInternalData.viewsMap)
  {
    assert(viewPair.second->initialize());
  }

  editorInternalData.currentView = sdfEditorView;
  
  return TRUE;
}

static void shutdown(Application* app)
{
  for(auto viewPair: editorInternalData.viewsMap)
  {
    viewPair.second->shutdown();
    destroyView(viewPair.second);
  }

  editorInternalData.viewsMap.clear();
}

static void update(Application* app, float64 delta)
{
  editorInternalData.currentView->update(delta);
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

  
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(screenWidth, screenHeight));
  ImGui::Begin(editorInternalData.currentView->name.c_str(), nullptr, viewWindowFlags);
  editorInternalData.currentView->draw(delta);
  ImGui::End();
}

static void processInput(Application* app, const EventData& eventData, void* sender)
{
  editorInternalData.currentView->processInput(eventData, sender);
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
