#include <vector>
#include <unordered_map>

#include <memory_manager.h>

#include "window_manager.h"

using std::vector;
using std::string;
using std::unordered_map;

struct WindowManager
{
  unordered_map<string, Window*> windowsMap;
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
    freeWindow(pair.second);
  }

  manager->windowsMap.clear();
}

void windowManagerAddWindow(WindowManager* manager, Window* window, bool8 initialize)
{
  string windowID = windowGetIdentifier(window);
  assert(manager->windowsMap.find(windowID) == manager->windowsMap.end() && "Same window cannot be added twice!");
  manager->windowsMap[windowID] = window;

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
    freeWindow(windowIt->second);
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

  return windowIt->second;
}

// TODO: std::vector<Window*> windowManagerGetWindows(WindowManager* manager);

void windowManagerDraw(WindowManager* manager, float64 delta)
{
  for(auto pair: manager->windowsMap)
  {
    if(windowIsVisible(pair.second))
    {
      drawWindow(pair.second, delta);
    }
  }
}

void windowManagerUpdate(WindowManager* manager, float64 delta)
{
  vector<Window*> windowsToRemove;
  
  for(auto pair: manager->windowsMap)
  {
    Window* window = pair.second;
    
    if(windowIsOpen(window) == TRUE)
    {
      updateWindow(pair.second, delta);
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
    processInputWindow(pair.second, eventData, sender);
  }
}
