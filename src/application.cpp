#include "memory_manager.h"

#include "application.h"

struct Application
{

  uint32      width;
  uint32      height;
  const char* name;
};

static Application application;

bool8 initApplication(uint32 width, uint32 height, const char* appName)
{
  application.width = width;
  application.height = height;
  application.name = appName;
  
  LOG_INFO("Application \"%s\" has been initialized successfully!", appName);
  LOG_WARNING("Application \"%s\" has been initialized successfully!", appName);
  LOG_ERROR("Application \"%s\" has been initialized successfully!", appName);
  LOG_VERBOSE("Application \"%s\" has been initialized successfully!", appName);  
  return TRUE;
}

void shutdownApplication()
{
  
}

void runApplication()
{
  
}
