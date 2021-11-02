#include <vector>
#include <unordered_map>

#include <memory_manager.h>

#include "window_manager.h"

using std::vector;
using std::string;
using std::unordered_map;

struct WindowData
{
  Window* window;
  bool8 visible;
};

struct WindowManager
{
  unordered_map<string, WindowData> windowsMap;
};

bool8 createWindowManager(WindowManager** outWindowManager)
{
  *outWindowManager = engineAllocObject<WindowManager>(MEMORY_TYPE_GENERAL);
  
  return TRUE;
}

void destroyWindowManager(WindowManager* manager)
{
  for(auto pair: manager->windowsMap)
  {
    freeWindow(pair.second.window);
  }

  manager->windowsMap.clear();
}

void windowManagerAddWindow(WindowManager* manager, Window* window, bool8 initialize)
{
  string windowID = windowGetIdentifier(window);
  assert(manager->windowsMap.find(windowID) == manager->windowsMap.end() && "Same window cannot be added twice!");
  manager->windowsMap[windowID] = {window, TRUE};

  if(initialize == TRUE)
  {
    initWindow(window);
  }
}

bool8 windowManagerRemoveWindow(WindowManager* manager, Window* window, bool8 free)
{
  return windowManagerRemoveWindow(manager, windowGetIdentifier(window), free);
}

bool8 windowManagerRemoveWindow(WindowManager* manager, const std::string& identifier, bool8 free)
{
  auto windowIt = manager->windowsMap.find(identifier);
  if(windowIt == manager->windowsMap.end())
  {
    return FALSE;
  }

  if(free == TRUE)
  {
    freeWindow(windowIt->second.window);
  }
  
  manager->windowsMap.erase(windowIt);
  return TRUE;
}

bool8 windowManagerHasWindow(WindowManager* manager, const std::string& identifier)
{
  return manager->windowsMap.find(identifier) != manager->windowsMap.end();
}

Window* windowManagerGetWindow(WindowManager* manager, const std::string& identifier)
{
  auto windowIt = manager->windowsMap.find(identifier);
  if(windowIt == manager->windowsMap.end())
  {
    return nullptr;
  }

  return windowIt->second.window;
}

// TODO: std::vector<Window*> windowManagerGetWindows(WindowManager* manager);

void windowManagerToggleWindow(WindowManager* manager, Window* window)
{
  windowManagerToggleWindow(manager, windowGetIdentifier(window));
}

void windowManagerToggleWindow(WindowManager* manager, const std::string& identifier)
{
  auto windowIt = manager->windowsMap.find(identifier);
  if(windowIt != manager->windowsMap.end())
  {
    windowIt->second.visible = !windowIt->second.visible;
  }
}

bool8 windowManagerIsWindowVisible(WindowManager* manager, Window* window)
{
  return windowManagerIsWindowVisible(manager, windowGetIdentifier(window));
}

bool8 windowManagerIsWindowVisible(WindowManager* manager, const std::string& identifier)
{
  auto windowIt = manager->windowsMap.find(identifier);
  if(windowIt == manager->windowsMap.end())
  {
    return FALSE;
  }

  return windowIt->second.visible;
}

void windowManagerDraw(WindowManager* manager, float64 delta)
{
  for(auto pair: manager->windowsMap)
  {
    if(pair.second.visible == TRUE)
    {
      drawWindow(pair.second.window, delta);
    }
  }
}

void windowManagerUpdate(WindowManager* manager, float64 delta)
{
  vector<Window*> windowsToRemove;
  
  for(auto pair: manager->windowsMap)
  {
    Window* window = pair.second.window;
    
    if(windowIsOpen(window) == TRUE)
    {
      updateWindow(pair.second.window, delta);
    }
    else
    {
      windowsToRemove.push_back(window);
    }
  }

  for(Window* window: windowsToRemove)
  {
    windowManagerRemoveWindow(manager, window);
  }
}

void windowManagerProcessInput(WindowManager* manager, const EventData& eventData, void* sender)
{
  for(auto pair: manager->windowsMap)
  {
    processInputWindow(pair.second.window, eventData, sender);
  }
}
