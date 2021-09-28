#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "event_system.h"
#include "memory_manager.h"
#include "lua/lua_system.h"

#include "application.h"
#include "game_framework.h"

struct Application
{
  bool8       initialized;
  uint32      width;
  uint32      height;
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

  triggerEvent(eventData.type, eventData);
  pushEvent(eventData.type, eventData);    
}

static void gcursorPosCallback(GLFWwindow* window, float64 xpos, float64 ypos)
{
  EventData eventData = {};
  eventData.type = EVENT_TYPE_CURSOR_MOVED;
  eventData.f32[0] = (float32)xpos;
  eventData.f32[1] = (float32)ypos;

  triggerEvent(eventData.type, eventData);
  pushEvent(eventData.type, eventData);
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

  triggerEvent(eventData.type, eventData);
  pushEvent(eventData.type, eventData);  
}

static void gwindowResizedCallback(GLFWwindow* window, int width, int height)
{
  application.width = (uint32)width;
  application.height = (uint32)height;

  EventData eventData = {};
  eventData.type = EVENT_TYPE_WINDOW_RESIZED;
  eventData.i32[0] = width;
  eventData.i32[1] = height;

  triggerEvent(eventData.type, eventData);
  pushEvent(eventData.type, eventData);  
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
  gladLoadGL();
  
  glfwSetKeyCallback(application.window, gkeyCallback);
  glfwSetCursorPosCallback(application.window, gcursorPosCallback);
  glfwSetMouseButtonCallback(application.window, gmouseButtonCallback);
  
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
  
  return TRUE;
}

static void shutdownImGUI()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

static bool8 initApplication()
{
  if(application.initialized)
  {
    LOG_ERROR("Application cannot be initialized twice!");
    return FALSE;
  }
  
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
  /** --- Event system initialization -------------------------------------- */
  if(initEventSystem() == FALSE)
  {
    LOG_ERROR("Cannot initialize event system!");
    return FALSE;
  }
  else
  {
    LOG_INFO("Event system has been initialized successfully!");
  }
  
  /** --- GLFW initialization ---------------------------------------------- */
  if(initGLFW() == FALSE)
  {
    LOG_ERROR("Cannot initialize GLFW!");
    return FALSE;
  }
  else
  {
    LOG_INFO("GLFW has been initialized successfully!");
  }

  /** --- ImGUI initialization --------------------------------------------- */
  if(initImGUI() == FALSE)
  {
    LOG_ERROR("Cannot initialize ImGUI!");
    return FALSE;
  }
  else
  {
    LOG_INFO("ImGUI has been initialized successfully!");
  }

  /** --- Lua system initialization ---------------------------------------- */
  if(initializeLuaSystem() == FALSE)
  {
    LOG_ERROR("Cannot initialize Lua system!");
    return FALSE;
  }
  else
  {
    LOG_INFO("Lua system has been initialized successfully!");
  }

  
  /** --- Game initialization ---------------------------------------------- */

  if(!game.initialize(&application))
  {
    LOG_ERROR("Cannot initialize game!");
    return FALSE;
  }
  else
  {
    LOG_INFO("Game \"%s\" has been initialized successfully!", application.name);
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
  shutdownLuaSystem();
  shutdownImGUI();
  shutdownGLFW();

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
  
  float64 prevAbsoluteTime = 0.0;
  while(!glfwWindowShouldClose(application.window))
  {
    float64 curAbsoluteTime = glfwGetTime();
    float64 delta = curAbsoluteTime - prevAbsoluteTime;
    prevAbsoluteTime = curAbsoluteTime;

    processInputApplication();
    updateApplication(delta);
    drawApplication(delta);
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

