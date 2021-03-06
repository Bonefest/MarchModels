#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "stopwatch.h"
#include "scheduler.h"
#include "event_system.h"
#include "image_manager.h"
#include "shader_manager.h"
#include "memory_manager.h"
#include "lua/lua_system.h"
#include "renderer/renderer.h"
#include "assets/assets_manager.h"
#include "assets/materials_atlas_system.h"

#include "application.h"
#include "game_framework.h"

using namespace march;

struct Application
{
  bool8       initialized;
  uint32      width;
  uint32      height;
  uint32      FPS;
  float64     frameTime;
  bool8       fixedFPS = FALSE;
  const char* name;

  GLFWwindow* window;
  ImGuiIO* imguiIO;
};

static Application application;
static GameFramework game;

static void gerrorCallback(int error, const char* description)
{
  LOG_ERROR("[%d] Error from GLFW: \"%s\"", description);
}

static void gkeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if(action == GLFW_REPEAT)
  {
    /** Repeat actions are not required */
    return;
  }
  
  EventData eventData = {};
  eventData.type = (action == GLFW_PRESS) ? EVENT_TYPE_KEY_PRESSED : EVENT_TYPE_KEY_RELEASED;
  eventData.i32[0] = key;
  eventData.i32[1] = scancode;
  eventData.i32[2] = action;
  eventData.i32[3] = mods;

  triggerEvent(eventData);
  pushEvent(eventData);    
}

static void gcursorPosCallback(GLFWwindow* window, float64 xpos, float64 ypos)
{
  EventData eventData = {};
  eventData.type = EVENT_TYPE_CURSOR_MOVED;
  eventData.f32[0] = (float32)xpos;
  eventData.f32[1] = (float32)ypos;

  triggerEvent(eventData);
  pushEvent(eventData);
}

static void gmouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
  if(action == GLFW_REPEAT)
  {
    /** Repeat actions are not required */
    return;
  }
  
  EventData eventData = {};
  eventData.type = (action == GLFW_PRESS) ? EVENT_TYPE_BUTTON_PRESSED : EVENT_TYPE_BUTTON_RELEASED;
  eventData.i32[0] = button;
  eventData.i32[1] = action;
  eventData.i32[2] = mods;

  triggerEvent(eventData);
  pushEvent(eventData);  
}

static void gscrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  EventData eventData = {};
  eventData.type = EVENT_TYPE_SCROLL_INPUT;
  eventData.f32[0] = xoffset;
  eventData.f32[1] = yoffset;
  
  triggerEvent(eventData);
  pushEvent(eventData);    
}

static void gwindowResizedCallback(GLFWwindow* window, int width, int height)
{
  application.width = (uint32)width;
  application.height = (uint32)height;

  EventData eventData = {};
  eventData.type = EVENT_TYPE_WINDOW_RESIZED;
  eventData.i32[0] = width;
  eventData.i32[1] = height;

  triggerEvent(eventData);
  pushEvent(eventData);  
}

static void openglErrorsCallback(GLenum source,
                                 GLenum type,
                                 GLuint id,
                                 GLenum severity,
                                 GLsizei length,
                                 const GLchar* message,
                                 const void* userParam)
{
  if(GL_DEBUG_SEVERITY_HIGH == severity || GL_DEBUG_SEVERITY_MEDIUM == severity)
  {
    if(GL_DEBUG_TYPE_ERROR == type)
    {
      LOG_ERROR("OpenGL: %s", message);
    }
    else
    {
      LOG_WARNING("OpenGL: %s", message);
    }
  }
}

static bool8 initGLFW()
{
  glfwSetErrorCallback(gerrorCallback);

  if(!glfwInit())
  {
    LOG_ERROR("glfwInit() failed!");
    return FALSE;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  application.window = glfwCreateWindow(application.width,
                                        application.height,
                                        application.name,
                                        NULL, NULL);
  if(application.window == nullptr)
  {
    LOG_ERROR("glfwCreateWindow() returned nullptr!");
    glfwTerminate();
    return FALSE;
  }

  glfwMakeContextCurrent(application.window);
  if(gladLoadGL() == 0)
  {
    LOG_ERROR("gladLoadGL() returned false (0)!");
    glfwTerminate();
    return FALSE;
  }

  glfwSwapInterval(1);
  
  glfwSetKeyCallback(application.window, gkeyCallback);
  glfwSetCursorPosCallback(application.window, gcursorPosCallback);
  glfwSetMouseButtonCallback(application.window, gmouseButtonCallback);
  glfwSetScrollCallback(application.window, gscrollCallback);
  
  #ifdef DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(openglErrorsCallback, NULL);
  #endif

  return TRUE;
}

static void shutdownGLFW()
{
  glfwDestroyWindow(application.window);
  glfwTerminate();
}

static bool8 initImGUI()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  application.imguiIO = &ImGui::GetIO();
  application.imguiIO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  (void)(*application.imguiIO);
  
  ImGui_ImplGlfw_InitForOpenGL(application.window, true);
  ImGui_ImplOpenGL3_Init("#version 150");

  application.imguiIO->Fonts->AddFontDefault();

  static const ImWchar iconsRange[] = {ICON_MIN_KI, ICON_MAX_KI};

  // NOTE: See https://github.com/ocornut/imgui/blob/master/docs/FONTS.md for details
  ImFontConfig iconsFontConfig;
  iconsFontConfig.MergeMode = true;
  iconsFontConfig.GlyphMinAdvanceX = 13.0f; // Default font size is 13.0 pixels wide.
                                            // This guarantees that glyph will be at least 13 pixels wide
  
  iconsFontConfig.GlyphOffset = float2(0, 2);
  application.imguiIO->Fonts->AddFontFromFileTTF("assets/fonts/kenney-icon-font.ttf",
                                                 13.0f,
                                                 &iconsFontConfig,
                                                 iconsRange);

  // TODO: Load this from a configuration?
  ImGuiStyle& style = ImGui::GetStyle();
  
  style.WindowPadding = float2(4.0f, 4.0f);
  style.WindowRounding = 12.0f;
  style.FrameRounding = 6.0f;
  style.TabRounding = 10.0f;
  style.WindowMenuButtonPosition = ImGuiDir_None;
  
  return TRUE;
}

static void shutdownImGUI()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

static bool8 preloadDefaultShaders()
{ 
  if(shaderManagerLoadShader(GL_VERTEX_SHADER, "shaders/triangle.vert", TRUE, "triangle.vert") == nullptr)
  {
    return FALSE;
  }

  return TRUE;
}

static bool8 initApplication()
{
  if(application.initialized)
  {
    LOG_ERROR("Application cannot be initialized twice!");
    return FALSE;
  }

  applicationSetFPS(60);
  
  if(!initializeGameFramework(&game))
  {
    LOG_ERROR("Failed to initialize a game!");
    return FALSE;
  }

  if(!game.extractSetupConfig(&application,
                              &application.width,
                              &application.height,
                              &application.name))
  {
    LOG_ERROR("Failed to extract initialize data from a game!");
    return FALSE;
  }

  /** --- Scheduler system initialization ----------------------------------- */
  if(initSchedulerSystem() == FALSE)
  {
    LOG_ERROR("Cannot initialize scheduler system!");
    return FALSE;
  }
  else
  {
    LOG_SUCCESS("Scheduler system has been initialize successfully!");
  }
  
  /** --- Event system initialization -------------------------------------- */
  if(initEventSystem() == FALSE)
  {
    LOG_ERROR("Cannot initialize event system!");
    return FALSE;
  }
  else
  {
    LOG_SUCCESS("Event system has been initialized successfully!");
  }
  
  /** --- GLFW initialization ---------------------------------------------- */
  if(initGLFW() == FALSE)
  {
    LOG_ERROR("Cannot initialize GLFW!");
    return FALSE;
  }
  else
  {
    LOG_SUCCESS("GLFW has been initialized successfully!");
  }

  /** --- ImGUI initialization --------------------------------------------- */
  if(initImGUI() == FALSE)
  {
    LOG_ERROR("Cannot initialize ImGUI!");
    return FALSE;
  }
  else
  {
    LOG_SUCCESS("ImGUI has been initialized successfully!");
  }

  /** --- Shader manager initialization ----------------------------------- */
  if(initShaderManager() == FALSE)
  {
    LOG_ERROR("Cannot initialize shader manager!");
    return FALSE;
  }
  else
  {
    LOG_SUCCESS("Shader manager has been initialized successfully!");
  }

  if(preloadDefaultShaders() == FALSE)
  {
    LOG_ERROR("Cannot preload default shaders!");
    return FALSE;
  }
  else
  {
    LOG_SUCCESS("Default shaders were preloaded successfully!");
  }
  
  /** --- Lua system initialization ---------------------------------------- */
  if(initializeLuaSystem() == FALSE)
  {
    LOG_ERROR("Cannot initialize Lua system!");
    return FALSE;
  }
  else
  {
    LOG_SUCCESS("Lua system has been initialized successfully!");
  }

  /** --- Assets manager initialization ------------------------------------ */
  if(initAssetsManager() == FALSE)
  {
    LOG_ERROR("Cannot initialize assets manager!");
    return FALSE;
  }
  else
  {
    LOG_SUCCESS("Assets manager has been initialized successfully!");
  }
  
  /** --- Rendering system initialization ---------------------------------- */  
  if(initializeRenderer() == FALSE)
  {
    LOG_ERROR("Cannot initialize rendering system!");
    return FALSE;
  }
  else
  {
    LOG_SUCCESS("Rendering system has been initialized successfully!");
  }

  /** --- Images manager initialization ------------------------------------ */
  if(initializeImageManager() == FALSE)
  {
    LOG_ERROR("Cannot initialize image manager!");
    return FALSE;
  }
  else
  {
    LOG_SUCCESS("Image manager has been initialized successfully!");
  }

  /** --- Materials atlas system initialization ---------------------------- */
  if(initializeMAS() == FALSE)
  {
    LOG_ERROR("Cannot initialize materials atlas system!");
    return FALSE;
  }
  else
  {
    LOG_SUCCESS("Materials atlas system has been initialized successfully!");
  }
  
  /** --- Game initialization ---------------------------------------------- */

  if(!game.initialize(&application))
  {
    LOG_ERROR("Cannot initialize game!");
    return FALSE;
  }
  else
  {
    LOG_SUCCESS("Game \"%s\" has been initialized successfully!", application.name);
  }
  
  application.initialized = TRUE;
  
  return TRUE;
}

static void shutdownApplication()
{
  if(!application.initialized)
  {
    LOG_ERROR("Cannot shutdown an uninitialized application!");
    return;
  }
     
  game.shutdown(&application);
  shutdownMAS();
  shutdownImageManager();
  shutdownRenderer();
  shutdownAssetsManager();
  shutdownLuaSystem();
  shutdownShaderManager();
  shutdownImGUI();
  shutdownGLFW();
  shutdownEventSystem();
  shutdownSchedulerSystem();

  application.initialized = FALSE;
}

static void processInputApplication()
{
  glfwPollEvents();  
  
  void* sender = nullptr;
  EventData eventData;
  while(pollEvent(&eventData, &sender) == TRUE)
  {
    game.processInput(&application, eventData, sender);
  }
}

static void updateApplication(float64 delta)
{
  schedulerUpdate(delta);
  masUpdate();
  game.update(&application, delta);
}


static void drawApplication(float64 delta)
{
  glViewport(0, 0, application.width, application.height);
  glClear(GL_COLOR_BUFFER_BIT);
  
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  game.draw(&application, delta);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  glfwSwapBuffers(application.window);
}

static void startApplicationLoop()
{
  glfwSetTime(0.0);

  Stopwatch mainLoopStopwatch;
  while(!glfwWindowShouldClose(application.window))
  {
    static float64 elapsedTime = 0.0f;
    float64 delta = mainLoopStopwatch.restart().asSecs();
    elapsedTime += delta;
    
    if(true)//elapsedTime >= application.frameTime)
    {
      delta = application.fixedFPS == TRUE ? application.frameTime : elapsedTime;
      
      processInputApplication();
      updateApplication(delta);
      drawApplication(delta);

      elapsedTime = 0.0f;
    }
  }
}

bool8 startApplication()
{
  if(!initApplication())
  {
    return FALSE;
  }

  startApplicationLoop();
  shutdownApplication();
  
  return TRUE;
}

void runApplication()
{

}

uint32 applicationGetScreenWidth()
{
  return application.width;
}

uint32 applicationGetScreenHeight()
{
  return application.height;
}

const char* applicationGetName()
{
  return application.name;
}

GLFWwindow* applicationGetWindow()
{
  return application.window;
}

void applicationSetFPS(uint32 FPS)
{
  assert(FPS > 0);
  
  application.FPS = FPS;
  application.frameTime = 1.0f / float32(FPS);
}

uint32 applicationGetFPS()
{
  return application.FPS;
}

void applicationSetFPSFixed(bool8 fixed)
{
  application.fixedFPS = fixed;
}

bool8 applicationIsFPSFixed()
{
  return application.fixedFPS;
}
