#include <logging.h>
#include <memory_manager.h>

#include "window.h"

struct Window
{
  WindowInterface interface;
  std::string identifier;
  
  bool8 initialized;
  void* internalData;
};

bool8 allocateWindow(WindowInterface interface, const std::string& identifier, Window** outWindow)
{
  *outWindow = engineAllocObject<Window>(MEMORY_TYPE_GENERAL);
  (*outWindow)->interface = interface;
  (*outWindow)->identifier = identifier;  
  (*outWindow)->initialized = FALSE;
  (*outWindow)->internalData = nullptr;
  
  return TRUE;
}

void freeWindow(Window* window)
{
  if(window->initialized == TRUE)
  {
    window->interface.shutdown(window);
  }

  engineFreeObject(window, MEMORY_TYPE_GENERAL);
}

bool8 initializeWindow(Window* window)
{
  if(window->initialized == TRUE)
  {
    LOG_ERROR("Attempt to initiailize window twice!");
    return FALSE;
  }

  window->initialized = window->interface.initialize(window);
  return window->initialized;
}

void updateWindow(Window* window, float64 delta)
{
  window->interface.update(window, delta);
}

void drawWindow(Window* window, float64 delta)
{
  window->interface.draw(window, delta);
}

void processInputWindow(Window* window, const EventData& eventData, void* sender)
{
  window->interface.processInput(window, eventData, sender);
}

const std::string& windowGetIdentifier(Window* window)
{
  return window->identifier;
}

void windowSetInternalData(Window* window, void* internalData)
{
  window->internalData = internalData;
}

void* windowGetInternalData(Window* window)
{
  return window->internalData;
}
