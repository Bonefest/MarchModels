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
  unordered_map<string, WindowPtr> windowsMap;
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

  data.windowsMap.clear();
}

void windowManagerAddWindow(WindowPtr window, bool8 initialize)
{
  string windowID = windowGetIdentifier(window);
  assert(data.windowsMap.find(windowID) == data.windowsMap.end() && "Same window cannot be added twice!");
  data.windowsMap[windowID] = window;

  if(initialize == TRUE)
  {
    initWindow(window);
  }
}

bool8 windowManagerRemoveWindow(Window* window)
{
  return windowManagerRemoveWindow(windowGetIdentifier(window));
}

bool8 windowManagerRemoveWindow(const std::string& identifier)
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

  data.windowsMap.erase(windowIt);
  return TRUE;
}

bool8 windowManagerHasWindow(const std::string& identifier)
{
  return data.windowsMap.find(identifier) != data.windowsMap.end();
}

WindowPtr windowManagerGetWindow(const std::string& identifier)
{
  auto windowIt = data.windowsMap.find(identifier);
  if(windowIt == data.windowsMap.end())
  {
    return WindowPtr(nullptr);
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
  vector<WindowPtr> windowsToRemove;
  
  for(auto pair: data.windowsMap)
  {
    WindowPtr window = pair.second;
    
    if(windowIsOpen(window) == TRUE)
    {
      updateWindow(pair.second, delta);
    }
    else
    {
      windowsToRemove.push_back(window);
    }
  }

  for(WindowPtr window: windowsToRemove)
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
