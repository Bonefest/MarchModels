#include <imgui/imgui.h>

#include <utils.h>
#include <stopwatch.h>
#include <application.h>
#include <cvar_system.h>
#include <memory_manager.h>
#include <renderer/renderer.h>
#include <samplers/center_sampler.h>
#include <ray_integrators/debug_ray_integrator.h>

#include "editor.h"
#include "ui_utils.h"
#include "view_window.h"

using namespace march;

DECLARE_CVAR(editor_ViewWindow_MouseSensitivity, 0.01f);
DECLARE_CVAR(editor_ViewWindow_CameraSpeed, 10.0f);

enum ViewControlMode
{
  VIEW_CONTROL_MODE_NONE,  
  VIEW_CONTROL_MODE_HOLD,
  VIEW_CONTROL_MODE_FIXED  
};

struct ViewSettingsWindowData;
struct ViewWindowData
{
  ImageIntegrator* integrator;
  RenderingParameters renderingParameters;
  
  float32 maxFPS;
  Stopwatch lifetimeStopwatch;
  Stopwatch refreshStopwatch;
  Time refreshPeriod;

  ViewControlMode controlMode;
  bool8 requestedRedrawImage;

  WindowPtr settingsWindow;  
};

static bool8 initializeViewWindow(Window* window)
{
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);
  
  Time initTime = Time::current();
  data->lifetimeStopwatch.setTimepoint(initTime);
  data->refreshStopwatch.setTimepoint(initTime);
  
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
  GLFWwindow* glfwWindow = applicationGetWindow();  
  
  if(data->controlMode != VIEW_CONTROL_MODE_NONE)
  {
    const static float32& cameraSpeed = CVarSystemReadFloat("editor_ViewWindow_CameraSpeed");
    float32 finalCameraSpeed = cameraSpeed;
    if(glfwGetKey(glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
      finalCameraSpeed *= 2.0f;
    }
    else if(glfwGetKey(glfwWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
      finalCameraSpeed *= 0.5f;
    }
    
    Camera* camera = imageIntegratorGetCamera(data->integrator);
    
    float3 camPosition = cameraGetPosition(camera);
    float4x4 camWorldBasis = cameraGetCameraWorldMat(camera);

    if(glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
    {
      camPosition += camWorldBasis[0].xyz() * finalCameraSpeed * float32(delta);
    }
    else if(glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
    {
      camPosition -= camWorldBasis[0].xyz() * finalCameraSpeed * float32(delta);
    }

    if(glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
    {
      camPosition += camWorldBasis[2].xyz() * finalCameraSpeed * float32(delta);
    }
    else if(glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
    {
      camPosition -= camWorldBasis[2].xyz() * finalCameraSpeed * float32(delta);
    }

    if(glfwGetKey(glfwWindow, GLFW_KEY_E) == GLFW_PRESS)
    {
      camPosition += float3(0.0f, 1.0f, 0.0f) * finalCameraSpeed * float32(delta);
    }
    else if(glfwGetKey(glfwWindow, GLFW_KEY_Q) == GLFW_PRESS)
    {
      camPosition -= float3(0.0f, 1.0f, 0.0f) * finalCameraSpeed * float32(delta);
    }    

    cameraSetPosition(camera, camPosition);
  }
}

static void updateViewWindowSize(Window* window, uint2 size)
{
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);
  imageIntegratorSetSize(data->integrator, size);
}

static void drawViewWindow(Window* window, float64 delta)
{
  const static uint32& culledObjectsCounter = CVarSystemReadUint("engine_RasterizationStatistics_LastFrameCulledObjects");  
  
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);
  ImGuiStyle& style = ImGui::GetStyle();
  
  Scene* currentScene = editorGetCurrentScene();
  imageIntegratorSetScene(data->integrator, currentScene);

  bool8 newFrameTicked = data->refreshStopwatch.isPaused() == FALSE &&
    data->refreshStopwatch.getElapsedTime() > data->refreshPeriod;
  
  if((data->requestedRedrawImage == TRUE || newFrameTicked == TRUE) && currentScene != nullptr)
  {
    data->renderingParameters.time = data->lifetimeStopwatch.getElapsedTime().asSecs();
    imageIntegratorExecute(data->integrator, data->renderingParameters);
    data->refreshStopwatch.restart();

    data->requestedRedrawImage = FALSE;
  }

  Film* film = imageIntegratorGetFilm(data->integrator);
  uint2 filmSize = filmGetSize(film);

  float2 windowSize = ImGui::GetWindowContentAreaSize();
  if(windowSize.x != filmSize.x || windowSize.y != filmSize.y)
  {
    updateViewWindowSize(window, uint2(windowSize.x, windowSize.y));
  }

  if(currentScene != nullptr)
  {
    float2 initialCursorPos = ImGui::GetCursorPos();
    float2 avalReg = ImGui::GetContentRegionAvail();

    ImGui::Image((void*)filmGetGLHandle(film),
                 float2(filmSize.x, filmSize.y), float2(0.0, 1.0), float2(1.0, 0.0));

    ImGui::SetCursorPos(initialCursorPos);

    if(ImGui::Button(ICON_KI_RELOAD_INVERSE"##view"))
    {
      data->lifetimeStopwatch.restart();
      data->refreshStopwatch.restart();
    }

    ImGui::SameLine();            

    if(ImGui::Button(ICON_KI_BACKWARD"##view"))
    {
      float64 totalSecs = data->lifetimeStopwatch.getTimepoint().asSecs();
      float64 currentSecs = Time::current().asSecs();

      // NOTE: Determine how much time to add (we need to limit it, so that elapsed time is not
      // overcome, i.e always greater than 0)
      float64 addSecs = min(data->lifetimeStopwatch.getElapsedTime().asSecs(), 1.0f);

      // NOTE: Shift timepoint 0.0-1.0 second(s) forward, so that total elapsed time is decreased.
      data->lifetimeStopwatch.setTimepoint(Time::secs(min(totalSecs + addSecs, currentSecs)));
    }

    ImGui::SameLine();

    if(data->lifetimeStopwatch.isPaused())
    {
      if(ImGui::Button(ICON_KI_CARET_RIGHT"##view"))
      {
        data->lifetimeStopwatch.unpause();
        data->refreshStopwatch.unpause();
      }
    }
    else
    {
      if(ImGui::Button(ICON_KI_PAUSE"##view"))
      {
        data->lifetimeStopwatch.pause();
        data->refreshStopwatch.pause();
      }
    }

    ImGui::SameLine();      

    if(ImGui::Button(ICON_KI_FORWARD"##view"))
    {
      float64 totalSecs = data->lifetimeStopwatch.getTimepoint().asSecs();

      // NOTE: Shift timepoint one second backward, so that total elapsed time is increased.
      data->lifetimeStopwatch.setTimepoint(Time::secs(totalSecs - 1.0f));
    }

    ImGui::SameLine();

    if(ImGui::Button(ICON_KI_PENCIL"##view"))
    {
      data->requestedRedrawImage = TRUE;
    }
    
    static float32 cogButtonWidth = 10.0f;
    ImGui::SameLine(windowSize.x - cogButtonWidth - style.FramePadding.x);

    bool8 cogPressed = ImGui::Button(ICON_KI_COG"##view");
    cogButtonWidth = ImGui::GetItemRectSize().x;

    if(cogPressed == TRUE && data->settingsWindow == nullptr)
    {
      Window* settingsWindow = nullptr;
      assert(createViewSettingsWindow(window, &settingsWindow));
      data->settingsWindow = WindowPtr(settingsWindow);
      
      windowManagerAddWindow(data->settingsWindow);      
    }

    char shortInfoBuf[255];
    sprintf(shortInfoBuf, "Time: %.2f | FPS: %u", data->lifetimeStopwatch.getElapsedTime().asSecs(), 0);
    float32 textWidth = ImGui::CalcTextSize(shortInfoBuf).x;

    ImGui::SameLine(windowSize.x - textWidth - cogButtonWidth - 2.0 * style.FramePadding.x);
    ImGui::Text("%s", shortInfoBuf);

    ImGui::SetCursorPos(initialCursorPos + float2(0.0f, avalReg.y - ImGui::GetFontSize()));
    ImGui::Text("Culled objects: %d", culledObjectsCounter);
  }
  else
  {
    ImGui::Text("Nothing to view: scene is not selected!");
  }

}

static void processInputViewWindow(Window* window,
                                   const EventData& eventData,
                                   void* sender)
{
  ImGuiIO& io = ImGui::GetIO();  
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);
  GLFWwindow* glfwWindow = applicationGetWindow();
  Camera* camera = imageIntegratorGetCamera(data->integrator);

  static bool8 ignoreFirstFrame = FALSE;
  
  if(eventData.type == EVENT_TYPE_BUTTON_PRESSED)
  {
    if(eventData.i32[0] == GLFW_MOUSE_BUTTON_RIGHT &&
       windowIsFocused(window) == TRUE &&
       windowIsHovered(window) == TRUE)
    {
      ignoreFirstFrame = TRUE;
      
      data->controlMode = VIEW_CONTROL_MODE_HOLD;
      io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
      glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      glfwSetCursorPos(glfwWindow, 0.0, 0.0);
    }
  }
  else if(eventData.type == EVENT_TYPE_BUTTON_RELEASED)
  {
    if(eventData.i32[0] == GLFW_MOUSE_BUTTON_RIGHT && data->controlMode == VIEW_CONTROL_MODE_HOLD)
    {
      data->controlMode = VIEW_CONTROL_MODE_NONE;      
      io.ConfigFlags &= ~(ImGuiConfigFlags_NoMouse);
      glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);      
    }
  }
  else if(eventData.type == EVENT_TYPE_KEY_PRESSED)
  {
    if(eventData.i32[0] == GLFW_KEY_F && data->controlMode == VIEW_CONTROL_MODE_HOLD)
    {
      data->controlMode = VIEW_CONTROL_MODE_FIXED;
    }
    else if(eventData.i32[0] == GLFW_KEY_ESCAPE && data->controlMode == VIEW_CONTROL_MODE_FIXED)
    {
      data->controlMode = VIEW_CONTROL_MODE_NONE;      
      io.ConfigFlags &= ~(ImGuiConfigFlags_NoMouse);
      glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
  }
  else if(eventData.type == EVENT_TYPE_CURSOR_MOVED)
  {
    if(data->controlMode != VIEW_CONTROL_MODE_NONE && ignoreFirstFrame == FALSE)
    {
      const static float32& sensitivity = CVarSystemReadFloat("editor_ViewWindow_MouseSensitivity");

      float32 dx = eventData.f32[0] * sensitivity;
      float32 dy = eventData.f32[1] * sensitivity;

      float3 eulerAngles = cameraGetEulerAngles(camera);
      eulerAngles.x += -dx;
      eulerAngles.y +=  dy;

      cameraSetOrientation(camera, eulerAngles);

      glfwSetCursorPos(glfwWindow, 0.0, 0.0);
    }

    ignoreFirstFrame = FALSE;
  }
  else if(eventData.type == EVENT_TYPE_SCROLL_INPUT)
  {
    if(data->controlMode != VIEW_CONTROL_MODE_NONE)
    {
      Camera* camera = imageIntegratorGetCamera(data->integrator);
    
      float3 camPosition = cameraGetPosition(camera);
      float4x4 camWorldBasis = cameraGetCameraWorldMat(camera);

      cameraSetPosition(camera, camPosition + camWorldBasis[2].xyz() * eventData.f32[1]);
    }
  }
}

static void viewWindowOnSettingsWindowShutdown(Window* window, Window* settingsWindow)
{
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);  
  data->settingsWindow = WindowPtr(nullptr);
}

static RenderingParameters& viewWindowGetRenderingParameters(Window* window)
{
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);
  return data->renderingParameters;  
}

static ImageIntegrator* viewWindowGetImageIntegrator(Window* window)
{
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);    
  return data->integrator;
}

void viewWindowSetMaxFPS(Window* window, float32 maxFPS)
{
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);
  data->maxFPS = maxFPS;
  data->refreshPeriod = Time::secs(1.0f / maxFPS);
}

float32 viewWindowGetMaxFPS(Window* window)
{
  ViewWindowData* data = (ViewWindowData*)windowGetInternalData(window);
  return data->maxFPS;
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
  assert(createFilm(uint2(1280, 720), &film));

  ImageIntegrator* imageIntegrator = nullptr;
  assert(createImageIntegrator(nullptr, sampler, rayIntegrator, film, camera, &imageIntegrator));
  imageIntegratorSetPixelGap(imageIntegrator, uint2(23, 23));
  
  ViewWindowData* data = engineAllocObject<ViewWindowData>(MEMORY_TYPE_GENERAL);
  data->integrator = imageIntegrator;
  data->settingsWindow = WindowPtr(nullptr);
  
  windowSetInternalData(*outWindow, data);
  viewWindowSetMaxFPS(*outWindow, 60.0f);

  return TRUE;
}

// ----------------------------------------------------------------------------
// View settings window
// ----------------------------------------------------------------------------

struct ViewSettingsWindowData
{
  Window* viewWindow;
};

static bool8 initializeViewSettingsWindow(Window* window)
{
  return TRUE;
}

static void shutdownViewSettingsWindow(Window* window)
{
  ViewSettingsWindowData* data = (ViewSettingsWindowData*)windowGetInternalData(window);    
  viewWindowOnSettingsWindowShutdown(data->viewWindow, window);
}

static void updateViewSettingsWindow(Window* window, float64 delta)
{

}

static void drawViewSettingsWindow(Window* window, float64 delta)
{
  ViewSettingsWindowData* data = (ViewSettingsWindowData*)windowGetInternalData(window);
  Window* viewWindow = data->viewWindow;
  ImageIntegrator* integrator = viewWindowGetImageIntegrator(data->viewWindow);

  // --- General settings -----------------------------------------------------
  if(ImGui::TreeNode("General settings"))
  {
    float32 maxFPS = viewWindowGetMaxFPS(viewWindow);
    ImGui::SliderFloat("Max FPS##ViewSettings", &maxFPS, 1.0f, 999.0f);

    viewWindowSetMaxFPS(viewWindow, maxFPS);

    ImGui::TreePop();
  }
  // --- Rendering settings ---------------------------------------------------
  if(ImGui::TreeNode("Rendering settings"))
  {
    RenderingParameters& params = viewWindowGetRenderingParameters(data->viewWindow);

    const static char* shadingModeLabels[] =
    {
      "Distances visualization",
      "IDs visualization",
      "Normals visualization",
      "Shadows visualization",
      "Simple shading",
      "PBR shading"
    };
    
    ImGui::Checkbox("Enable shadows", (bool*)&params.enableShadows);
    ImGui::SameLine();
    ImGui::Checkbox("Enable normals", (bool*)&params.enableNormals);
    ImGui::SameLine();
    ImGui::Checkbox("Show UI widgets", (bool*)&params.showUIWidgets);
    ImGui::SameLine();
    ImGui::Checkbox("Show Lights", (bool*)&params.showLights);
    
    ImGui::Combo("Shading mode", (int32*)&params.shadingMode, shadingModeLabels, ARRAY_SIZE(shadingModeLabels));
    ImGui::SliderFloat("Gamma", &params.gamma, 1.0f, 3.0f);
    ImGui::SliderInt("Rasterizations iterations count", (int*)&params.rasterItersMaxCount, 1, 512);
    ImGui::SliderInt("Shadow rasterizations iterations count", (int*)&params.shadowRasterItersMaxCount, 1, 512);    
    ImGui::SliderFloat("Intersection threshold", &params.intersectionThreshold, 0.0001f, 100.0f);

    uint2 minValue = uint2(0, 0), maxValue = uint2(128, 128);
    ImGui::SliderScalarN("Pixel gap", ImGuiDataType_U32, &params.pixelGap, 2, &minValue, &maxValue);    
    
    ImGui::TreePop();
  }

  // --- Render passes settings -----------------------------------------------
  if(ImGui::TreeNode("Render passes"))
  {
    const std::vector<RenderPass*>& passes = rendererGetPasses();

    for(RenderPass* pass: passes)
    {
      if(renderPassHasInputView(pass) == TRUE)
      {
        if(ImGui::TreeNode(renderPassGetName(pass)))
        {
          renderPassDrawInputView(pass);

          ImGui::TreePop();
        }
      }
    }

    ImGui::TreePop();
  }
  
  // --- Camera settings ------------------------------------------------------
  if(ImGui::TreeNode("Camera settings"))
  {
    Camera* camera = imageIntegratorGetCamera(integrator);
    float3 pos = cameraGetPosition(camera);
    float3 ori = cameraGetEulerAngles(camera);

    float32 yaw = ori.x;
    float32 pitch = ori.y;

    bool updateFrustum = (cameraUpdatesFrustum(camera) == TRUE ? true : false);
    
    if(ImGui::Checkbox("Update frustum", &updateFrustum))
    {
      cameraSetUpdateFrustum(camera, updateFrustum);
    }
    
    pushIconSmallButtonStyle();
    if(ImGui::Button(ICON_KI_RELOAD_INVERSE"##ReloadPosition"))
    {
      pos = float3(0.0f, 0.0f, -10.0f);
    }
    popIconSmallButtonStyle();
    ImGui::SameLine();
    ImGui::SliderFloat3("Position##Camera", &pos.x, -10.0f, 10.0f);

    pushIconSmallButtonStyle();
    if(ImGui::Button(ICON_KI_RELOAD_INVERSE"##ReloadYaw"))
    {
      yaw = 0.0f;
    }
    popIconSmallButtonStyle();
    ImGui::SameLine();    
    ImGui::SliderAngle("Yaw##Camera", &yaw, -180.0f, 180.0f);

    pushIconSmallButtonStyle();
    if(ImGui::Button(ICON_KI_RELOAD_INVERSE"##ReloadPitch"))
    {
      pitch = 0.0f;
    }
    popIconSmallButtonStyle();
    ImGui::SameLine();    
    ImGui::SliderAngle("Pitch##Camera", &pitch, -90.0f, 90.0f);

    cameraSetPosition(camera, pos);
    cameraSetOrientation(camera, yaw, pitch);
    
    ImGui::TreePop();
  }

}

static void processInputViewSettingsWindow(Window* window, const EventData& eventData, void* sender)
{

}

bool8 createViewSettingsWindow(Window* viewWindow, Window** outWindow)
{
  assert(viewWindow != nullptr);

  std::string identifier = windowGetIdentifier(viewWindow) + " settings";
  
  WindowInterface interface = {};
  interface.initialize = initializeViewSettingsWindow;
  interface.shutdown = shutdownViewSettingsWindow;
  interface.update = updateViewSettingsWindow;
  interface.draw = drawViewSettingsWindow;
  interface.processInput = processInputViewSettingsWindow;

  if(allocateWindow(interface, identifier, outWindow) == FALSE)
  {
    return FALSE;
  }
  
  ViewSettingsWindowData* data = engineAllocObject<ViewSettingsWindowData>(MEMORY_TYPE_GENERAL);
  data->viewWindow = viewWindow;

  windowSetInternalData(*outWindow, data);

  return TRUE;
}
