#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;

#include <logging.h>
#include <memory_manager.h>

#include <imgui/imgui.h>

#include "window.h"

struct Window
{
  WindowInterface interface;
  string identifier;

  float2 position;
  float2 size;
  ImGuiWindowFlags flags;
  
  bool8 initialized;
  bool open;
  bool8 paramsUpdated;
  bool8 visible;
  bool8 focused;
  bool8 hovered;
  unordered_map<ImGuiStyleVar, float2> styles;
  unordered_map<ImGuiStyleVar, float2> stylesInfluenceChildren;
  
  void* internalData;
};

static uint32 vectorStyles = (1 << ImGuiStyleVar_WindowPadding)    |
                             (1 << ImGuiStyleVar_WindowMinSize)    |
                             (1 << ImGuiStyleVar_WindowTitleAlign) |
                             (1 << ImGuiStyleVar_FramePadding)     |
                             (1 << ImGuiStyleVar_ItemSpacing)      |
                             (1 << ImGuiStyleVar_ItemInnerSpacing) |
                             (1 << ImGuiStyleVar_CellPadding)      |
                             (1 << ImGuiStyleVar_ButtonTextAlign)  |
                             (1 << ImGuiStyleVar_SelectableTextAlign);  

static bool8 isVectorStyle(ImGuiStyleVar style)
{
  return ((1 << (uint32)style) & vectorStyles) == (1 << (uint32)style);
}

bool8 allocateWindow(WindowInterface interface, const string& identifier, Window** outWindow)
{
  *outWindow = engineAllocObject<Window>(MEMORY_TYPE_GENERAL);
  (*outWindow)->interface = interface;
  (*outWindow)->identifier = identifier;
  (*outWindow)->flags = 0;
  (*outWindow)->initialized = FALSE;
  (*outWindow)->open = TRUE;
  (*outWindow)->paramsUpdated = TRUE;
  (*outWindow)->visible = TRUE;
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

      if(window->focused == TRUE)
      {
        ImGui::SetNextWindowFocus();
      }
      
      window->paramsUpdated = TRUE;
    }

    for(auto stylePair: window->stylesInfluenceChildren)
    {
      if(isVectorStyle(stylePair.first) == TRUE)
      {
        ImGui::PushStyleVar(stylePair.first, stylePair.second);
      }
      else
      {
        ImGui::PushStyleVar(stylePair.first, stylePair.second.x);
      }
    }

    for(auto stylePair: window->styles)
    {
      if(isVectorStyle(stylePair.first) == TRUE)
      {
        ImGui::PushStyleVar(stylePair.first, stylePair.second);
      }
      else
      {
        ImGui::PushStyleVar(stylePair.first, stylePair.second.x);
      }
    }
    
    ImGui::Begin(window->identifier.c_str(), &window->open, window->flags);

    ImGui::PopStyleVar(window->styles.size());
    
    window->position = ImGui::GetWindowPos();
    window->size = ImGui::GetWindowSize();
  }

  // TODO: Push focusing/hovering events
  window->focused = ImGui::IsWindowFocused() ? TRUE : FALSE;
  window->hovered = ImGui::IsWindowHovered() ? TRUE : FALSE;
  
  window->interface.draw(window, delta);
    
  if(window->interface.usesCustomDrawPipeline == FALSE)
  {
    ImGui::End();

    ImGui::PopStyleVar(window->stylesInfluenceChildren.size());
  }
}

void processInputWindow(Window* window, const EventData& eventData, void* sender)
{
  window->interface.processInput(window, eventData, sender);
}

void windowSetSize(Window* window, float2 size)
{
  window->size = size;
  window->paramsUpdated = FALSE;
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

void windowClose(Window* window)
{
  window->open = FALSE;
}

void windowSetOpen(Window* window, bool8 open)
{
  window->open = (open == TRUE ? true : false);
}

bool8 windowIsOpen(Window* window)
{
  return window->open ? TRUE : FALSE;
}

void windowSetVisible(Window* window, bool8 visible)
{
  window->visible = visible;
}

bool8 windowIsVisible(Window* window)
{
  return window->visible;
}

void windowSetFocused(Window* window, bool8 focused)
{
  window->focused = focused;
  window->paramsUpdated = FALSE;
}

bool8 windowIsFocused(Window* window)
{
  return window->focused;
}

void windowClearAllStyles(Window* window)
{
  window->styles.clear();
  window->stylesInfluenceChildren.clear();
}

void windowClearStyle(Window* window, ImGuiStyleVar style)
{
  if(window->styles.erase(style) == 0)
  {
    window->stylesInfluenceChildren.erase(style);
  }
}

void windowSetStyle(Window* window, ImGuiStyleVar style, float32 value, bool8 influenceChild)
{
  windowSetStyle(window, style, float2(value, 0), influenceChild);
}

float32 windowGetStyle(Window* window, ImGuiStyleVar style, bool8* influenceChild)
{
  return windowGetStyle2(window, style, influenceChild).x;
}

void windowSetStyle(Window* window, ImGuiStyleVar style, float2 value, bool8 influenceChild)
{
  if(influenceChild == TRUE)
  {
    // Attempt to remove style from another map, so that only one copy of a style exists    
    window->styles.erase(style);
    window->stylesInfluenceChildren[style] = value;
  }
  else
  {
    window->styles[style] = value;
  }
}

float2 windowGetStyle2(Window* window, ImGuiStyleVar style, bool8* influenceChild)
{
  auto styleIt = window->stylesInfluenceChildren.find(style);
  if(styleIt != window->stylesInfluenceChildren.end())
  {
    if(influenceChild != nullptr)
    {
      *influenceChild = TRUE;
    }
    
    return styleIt->second;
  }

  styleIt = window->styles.find(style);
  if(styleIt != window->styles.end())
  {
    if(influenceChild != nullptr)
    {
      *influenceChild = FALSE;
    }
    
    return styleIt->second;
  }

  LOG_WARNING("Requested style '%d' is not overriden!", (int)style);
  return float2();
}

void windowSetFlags(Window* window, ImGuiWindowFlags flags)
{
  window->flags = flags;
}

ImGuiWindowFlags windowGetFlags(Window* window)
{
  return window->flags;
}


const string& windowGetIdentifier(Window* window)
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
