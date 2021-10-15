#include <imgui/imgui.h>
#include <memory_manager.h>

#include "editor.h"
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
  
  Scene* currentScene = editorGetCurrentScene();
  imageIntegratorSetScene(data->integrator, currentScene);
  
  if(data->elapsedTime > data->timePerFrame && currentScene != nullptr)
  {
    imageIntegratorExecute(data->integrator, glfwGetTime());
    data->elapsedTime = 0.0f;
  }

  Film* film = imageIntegratorGetFilm(data->integrator);
  uint2 filmSize = filmGetSize(film);

  ImGui::Begin(windowGetIdentifier(window).c_str());

    ImGuiStyle& style = ImGui::GetStyle();
  
    ImVec2 windowSize = ImGui::GetWindowContentAreaSize();
    if(windowSize.x != filmSize.x || windowSize.y != filmSize.y)
    {
      updateViewWindowSize(window, uint2(windowSize.x, windowSize.y));
    }

    if(currentScene != nullptr)
    {
      ImVec2 initialCursorPos = ImGui::GetCursorPos();
      
      ImGui::Image((void*)filmGetGLTexture(film),
                   ImVec2(filmSize.x, filmSize.y), ImVec2(1.0f, 1.0f), ImVec2(0.0f, 0.0f));

      ImGui::SetCursorPos(initialCursorPos);

      ImGui::Button(ICON_KI_RELOAD_INVERSE"##view");

      ImGui::SameLine();            
      
      ImGui::Button(ICON_KI_BACKWARD"##view");

      ImGui::SameLine();
      
      ImGui::Button(ICON_KI_PAUSE"##view");
      
      ImGui::SameLine();      

      ImGui::Button(ICON_KI_FORWARD"##view");

      static float32 cogButtonWidth = 10.0f;
      ImGui::SameLine(windowSize.x - cogButtonWidth - style.FramePadding.x);

      bool8 cogPressed = ImGui::Button(ICON_KI_COG"##view");
      cogButtonWidth = ImGui::GetItemRectSize().x;

      if(cogPressed == TRUE)
      {
        ImGui::OpenPopup("view_settings_popup##view");
      }
      
      char shortInfoBuf[255];
      sprintf(shortInfoBuf, "Time: %.2f | FPS: %u", glfwGetTime(), 0);
      float32 textWidth = ImGui::CalcTextSize(shortInfoBuf).x;
      
      ImGui::SameLine(windowSize.x - textWidth - cogButtonWidth - 2.0 * style.FramePadding.x);
      ImGui::Text(shortInfoBuf);

      if(ImGui::BeginPopup("view_settings_popup##view"))
      {
        ImGui::Text("Time: %f", glfwGetTime());
        ImGui::SameLine();
        ImGui::Text("Max FPS: %f", data->maxFPS);
        ImGui::Button("Open full settings");
        
        ImGui::EndPopup();
      }
    }
    else
    {
      ImGui::Text("Nothing to view: scene is not selected!");
    }
  ImGui::End();

}

static void processInputViewWindow(Window* window,
                                   const EventData& eventData,
                                   void* sender)
{
  
}

bool8 createViewWindow(const std::string& identifier,
                       Sampler* sampler,
                       RayIntegrator* rayIntegrator,
                       Camera* camera,
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

  Film* film = nullptr;
  assert(createFilm(uint2(8, 8), &film));

  ImageIntegrator* imageIntegrator = nullptr;
  assert(createImageIntegrator(nullptr, sampler, rayIntegrator, film, camera, &imageIntegrator));
  imageIntegratorSetPixelGap(imageIntegrator, uint2(23, 23));
  
  ViewWindowData* data = engineAllocObject<ViewWindowData>(MEMORY_TYPE_GENERAL);
  data->integrator = imageIntegrator;
  data->elapsedTime = 0.0f;
  
  windowSetInternalData(*outWindow, data);
  
  viewWindowSetMaxFPS(*outWindow, 10.0f);

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
