#include <logging.h>
#include <memory_manager.h>

#include <imgui/imgui.h>

#include "window.h"

struct Window
{
  WindowInterface interface;
  std::string identifier;

  float2 position;
  float2 size;
  ImGuiWindowFlags flags;
  
  bool8 initialized;
  bool open;
  bool8 paramsUpdated;
  
  void* internalData;
};

bool8 allocateWindow(WindowInterface interface, const std::string& identifier, Window** outWindow)
{
  *outWindow = engineAllocObject<Window>(MEMORY_TYPE_GENERAL);
  (*outWindow)->interface = interface;
  (*outWindow)->identifier = identifier;
  (*outWindow)->flags = 0;
  (*outWindow)->initialized = FALSE;
  (*outWindow)->open = TRUE;
  (*outWindow)->paramsUpdated = TRUE;
  (*outWindow)->internalData = nullptr;

  EventData createEvent = {};
  createEvent.type = EVENT_TYPE_WINDOW_CREATED;
  createEvent.ptr[0] = *outWindow;

  pushEvent(createEvent);
  
  return TRUE;
}

void freeWindow(Window* window)
{
  if(window->initialized == TRUE)
  {
    window->interface.shutdown(window);
  }

  engineFreeObject(window, MEMORY_TYPE_GENERAL);

  EventData destroyEvent = {};
  destroyEvent.type = EVENT_TYPE_WINDOW_DESTROYED;
  destroyEvent.ptr[0] = window;
  
  pushEvent(destroyEvent);
}

bool8 initWindow(Window* window)
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
  if(window->interface.usesCustomDrawPipeline == FALSE)
  {
    if(window->paramsUpdated == FALSE)
    {
      ImGui::SetNextWindowPos(window->position);
      ImGui::SetNextWindowSize(window->size);
      
      window->paramsUpdated = TRUE;
    }
    
    ImGui::Begin(window->identifier.c_str(), &window->open, window->flags);

    window->position = ImGui::GetWindowPos();
    window->size = ImGui::GetWindowSize();
  }
  
  window->interface.draw(window, delta);
    
  if(window->interface.usesCustomDrawPipeline == FALSE)
  {
    ImGui::End();    
  }
}

void processInputWindow(Window* window, const EventData& eventData, void* sender)
{
  window->interface.processInput(window, eventData, sender);
}

void windowSetSize(Window* window, float2 size)
{
  window->size = size;
}

float2 windowGetSize(Window* window)
{
  return window->size;
}

void windowSetPosition(Window* window, float2 position)
{
  window->position = position;
}

float2 windowGetPosition(Window* window)
{
  return window->position;
}

void windowSetOpen(Window* window, bool8 open)
{
  window->open = (open == TRUE ? true : false);
}

bool8 windowIsOpen(Window* window)
{
  return window->open ? TRUE : FALSE;
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
