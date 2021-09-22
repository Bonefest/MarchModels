#if defined(ENABLE_FAR_GAME1)

#include <imgui/imgui.h>

#include <game_framework.h>

bool8 gameExtractSetupConfig(Application* app,
                             uint32* outScreenWidth,
                             uint32* outScreenHeight,
                             const char** outName)
{
  *outScreenWidth = 1280;
  *outScreenHeight = 720;
  *outName = "FAR Lab1";

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

void gameDraw(Application* app, float64 delta)
{
  static bool showDemoWindow = true;
  ImGui::ShowDemoWindow(&showDemoWindow);
}

void gameProcessInput(Application* app, const EventData& eventData, void* sender)
{

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
