#if defined(ENABLE_FAR_GAME1)

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <logging.h>
#include <application.h>
#include <game_framework.h>

struct GameInternalData
{
  const char* sideBarWindowName;
  const char* viewportWindowName;

  ImGuiID sideBarID;
  ImGuiID viewportID;
  
};

static GameInternalData gameData;

bool8 gameExtractSetupConfig(Application* app,
                             uint32* outScreenWidth,
                             uint32* outScreenHeight,
                             const char** outName)
{
  *outScreenWidth = 1280;
  *outScreenHeight = 720;
  *outName = "FAR Lab1";

  gameData.sideBarWindowName = "Sidebar";
  gameData.viewportWindowName = "Viewport";

  return TRUE;
}

bool8 gameInitialize(Application* app)
{

  return TRUE;
}

void gameShutdown(Application* app)
{

}

void gameUpdate(Application* app, float64 delta)
{

}

static void gameGenerateLayout(Application* app)
{
  uint32 screenWidth = applicationGetScreenWidth(), screenHeight = applicationGetScreenHeight();  
  ImGuiID mainNodeID = ImGui::GetID("Main");
  
  if(ImGui::DockBuilderGetNode(mainNodeID) == nullptr)
  {
    ImGui::DockBuilderAddNode(mainNodeID, ImGuiDockNodeFlags_DockSpace |
                              ImGuiDockNodeFlags_NoTabBar |
                              ImGuiDockNodeFlags_NoCloseButton);
    
    ImGui::DockBuilderSetNodeSize(mainNodeID, ImVec2(screenWidth, screenHeight));

    ImGuiID sideBarID, viewportID;

    /** DockBuilderSpitNode() splits given node on two parts */
    ImGui::DockBuilderSplitNode(mainNodeID, ImGuiDir_Left, 0.25f, &sideBarID, &viewportID);

    ImGui::DockBuilderDockWindow(gameData.sideBarWindowName, sideBarID);
    ImGui::DockBuilderDockWindow(gameData.viewportWindowName, viewportID);    

    ImGui::DockBuilderFinish(mainNodeID);

    gameData.sideBarID = sideBarID;
    gameData.viewportID = viewportID;
  }

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(screenWidth, screenHeight));

  static const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoBackground |
    ImGuiWindowFlags_NoDocking |
    ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_NoNavFocus;

  ImGui::Begin("Fundamentals of Augmented Reality | Assignment #1", nullptr, windowFlags);
  ImGui::DockSpace(mainNodeID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
  ImGui::End();
}

static void generateSidebarWindow(Application* app)
{
  ImGui::Begin(gameData.sideBarWindowName, nullptr);
  ImVec2 size = ImGui::GetWindowSize();
  ImGui::Text("Size x = %f, y = %f", size.x, size.y);
  ImGui::End();
}

static void generateViewportWindow(Application* app)
{
  ImGui::Begin(gameData.viewportWindowName, nullptr);
  ImGui::End();  
}

void gameDraw(Application* app, float64 delta)
{
  gameGenerateLayout(app);
  generateSidebarWindow(app);
  generateViewportWindow(app);
}

void gameProcessInput(Application* app, const EventData& eventData, void* sender)
{
  if(eventData.type == EVENT_TYPE_CURSOR_MOVED)
  {
    LOG_WARNING("Mouse moved: %f %f", eventData.f32[0], eventData.f32[1]);
  }
}

bool8 initializeGameFramework(GameFramework* outFramework)
{
  outFramework->extractSetupConfig = gameExtractSetupConfig;
  outFramework->initialize = gameInitialize;
  outFramework->shutdown = gameShutdown;
  outFramework->update = gameUpdate;
  outFramework->draw = gameDraw;
  outFramework->processInput = gameProcessInput;
  
  return TRUE;
}

#endif
