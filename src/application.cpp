#include "event_system.h"
#include "memory_manager.h"

#include "application.h"

struct Application
{
  uint32      width;
  uint32      height;
  const char* name;

  GLFWwindow* window;
};

static Application application;

static void gerrorCallback(int error, const char* description)
{
  LOG_ERROR("[%d] Error from GLFW: \"%s\"", description);
}

static void gkeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  /** trigger an event system */
}

static void gcursorPosCallback(GLFWwindow* window, float64 xpos, float64 ypos)
{
  /** trigger an event system */
}

static void gmouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
  /** trigger an event system */
}

static bool8 initGLFW()
{
  glfwSetErrorCallback(gerrorCallback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

  if(!glfwInit())
  {
    LOG_ERROR("glfwInit() failed!");
    return FALSE;
  }
  
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

  glfwSetKeyCallback(application.window, gkeyCallback);
  glfwSetCursorPosCallback(application.window, gcursorPosCallback);
  glfwSetMouseButtonCallback(application.window, gmouseButtonCallback);
  
  return TRUE;
}

bool8 initApplication(uint32 width, uint32 height, const char* appName)
{
  application.width = width;
  application.height = height;
  application.name = appName;

  if(initEventSystem() == FALSE)
  {
    LOG_ERROR("Cannot initialize event system!");
    return FALSE;
  }
  
  if(initGLFW() == FALSE)
  {
    LOG_ERROR("Cannot initialize GLFW!");
    return FALSE;
  }
  
  LOG_INFO("Application \"%s\" has been initialized successfully!", appName);
  
  return TRUE;
}

void shutdownApplication()
{
  
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

