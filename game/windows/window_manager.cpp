#include <vector>
#include <unordered_map>

#include <logging.h>
#include <memory_manager.h>

#include "window_manager.h"

using std::vector;
using std::string;
using std::unordered_map;

struct WindowManagerData
{
  bool8 initialized;
  unordered_map<string, Window*> windowsMap;
};

static WindowManagerData data;

bool8 initWindowManager()
{
  assert(data.initialized == FALSE);
  
  data.initialized = TRUE;
  return TRUE;
}

void shutdownWindowManager()
{
  assert(data.initialized == TRUE);
  
  for(auto pair: data.windowsMap)
  {
    freeWindow(pair.second);
  }

  data.windowsMap.clear();
}

void windowManagerAddWindow(Window* window, bool8 initialize)
{
  string windowID = windowGetIdentifier(window);
  assert(data.windowsMap.find(windowID) == data.windowsMap.end() && "Same window cannot be added twice!");
  data.windowsMap[windowID] = window;

  if(initialize == TRUE)
  {
    initWindow(window);
  }
}

bool8 windowManagerRemoveWindow(Window* window, bool8 free)
{
  return windowManagerRemoveWindow(windowGetIdentifier(window), free);
}

bool8 windowManagerRemoveWindow(const std::string& identifier, bool8 free)
{
  std::string trimmedId = identifier;
  std::size_t hashIdx = trimmedId.find("##");
  trimmedId = trimmedId.substr(0, hashIdx);

  LOG_INFO("Removing '%s' window", trimmedId.c_str());
  
  auto windowIt = data.windowsMap.find(identifier);
  if(windowIt == data.windowsMap.end())
  {
    return FALSE;
  }

  if(free == TRUE)
  {
    freeWindow(windowIt->second);
  }

  data.windowsMap.erase(windowIt);
  return TRUE;
}

bool8 windowManagerHasWindow(const std::string& identifier)
{
  return data.windowsMap.find(identifier) != data.windowsMap.end();
}

Window* windowManagerGetWindow(const std::string& identifier)
{
  auto windowIt = data.windowsMap.find(identifier);
  if(windowIt == data.windowsMap.end())
  {
    return nullptr;
  }

  return windowIt->second;
}

// TODO: std::vector<Window*> windowManagerGetWindows(WindowManager* manager);

void windowManagerDraw(float64 delta)
{
  for(auto pair: data.windowsMap)
  {
    if(windowIsVisible(pair.second))
    {
      drawWindow(pair.second, delta);
    }
  }
}

void windowManagerUpdate(float64 delta)
{
  vector<Window*> windowsToRemove;
  
  for(auto pair: data.windowsMap)
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
    windowManagerRemoveWindow(window);
  }
}

void windowManagerProcessInput(const EventData& eventData, void* sender)
{
  for(auto pair: data.windowsMap)
  {
    processInputWindow(pair.second, eventData, sender);
  }
}
