#include <imgui/imgui.h>
#include <memory_manager.h>

#include "view_window.h"

struct ViewWindowData
{
  ImageIntegrator* integrator;
  float32 maxFPS;
  float32 timePerFrame;
  float32 elapsedTime;
};

static bool8 initializeViewWindow(Window* window)
{
  return TRUE;
}

static void shutdownViewWindow(Window* window)
{
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);
  engineFreeMem(data, MEMORY_TYPE_GENERAL);
}

static void updateViewWindow(Window* window, float64 delta)
{
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);
  data->elapsedTime += delta;
}

static void updateViewWindowSize(Window* window, uint2 size)
{
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);
  imageIntegratorSetSize(data->integrator, size);
}

static void drawViewWindow(Window* window, float64 delta)
{
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);  
  if(data->elapsedTime > data->timePerFrame)
  {
    imageIntegratorExecute(data->integrator, glfwGetTime());
    data->elapsedTime = 0.0f;
  }

  Film* film = imageIntegratorGetFilm(data->integrator);
  uint2 filmSize = filmGetSize(film);

  ImGui::Begin(windowGetIdentifier(window).c_str());

  ImVec2 windowSize = ImGui::GetWindowContentAreaSize();
  if(windowSize.x != filmSize.x || windowSize.y != filmSize.y)
  {
    updateViewWindowSize(window, uint2(windowSize.x, windowSize.y));
  }
  
  ImGui::Image((void*)filmGetGLTexture(film),
               ImVec2(filmSize.x, filmSize.y), ImVec2(1.0f, 1.0f), ImVec2(0.0f, 0.0f));  
  ImGui::End();

}

static void processInputViewWindow(Window* window,
                                   const EventData& eventData,
                                   void* sender)
{
  
}

bool8 createViewWindow(const std::string& identifier,
                       ImageIntegrator* integrator,
                       float32 maxFPS,
                       Window** outWindow)
{
  WindowInterface interface = {};
  interface.initialize = initializeViewWindow;
  interface.shutdown = shutdownViewWindow;
  interface.update = updateViewWindow;
  interface.draw = drawViewWindow;
  interface.processInput = processInputViewWindow;

  if(allocateWindow(interface, identifier, outWindow) == FALSE)
  {
    return FALSE;
  }

  ViewWindowData* data = engineAllocObject<ViewWindowData>(MEMORY_TYPE_GENERAL);
  data->integrator = integrator;
  data->elapsedTime = 0.0f;

  windowSetInternalData(*outWindow, data);
  viewWindowSetMaxFPS(*outWindow, maxFPS);

  return TRUE;
}


void viewWindowSetMaxFPS(Window* window, float32 maxFPS)
{
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);
  data->maxFPS = maxFPS;
  data->timePerFrame = 1.0f / maxFPS;
}

float32 viewWindowGetMaxFPS(Window* window)
{
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);
  return data->maxFPS;
}
