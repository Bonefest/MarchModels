#include <imgui/imgui.h>

#include <memory_manager.h>
#include <assets/assets_manager.h>

#include "assets_list_window.h"

using std::vector;
using std::string;

struct AssetsListWindowData
{
  char pass;
};

static bool8 assetsListWindowInitialize(Window* window);
static void assetsListWindowShutdown(Window* window);
static void assetsListWindowUpdate(Window* window, float64 delta);
static void assetsListWindowDraw(Window* window, float64 delta);
static void assetsListWindowProcessInput(Window* window, const EventData& eventData, void* sender);

bool8 createAssetsListWindow(Window** outWindow)
{
  WindowInterface interface = {};
  interface.initialize = assetsListWindowInitialize;
  interface.shutdown = assetsListWindowShutdown;
  interface.update = assetsListWindowUpdate;
  interface.draw = assetsListWindowDraw;
  interface.processInput = assetsListWindowProcessInput;

  if(allocateWindow(interface, assetsListWindowGetIdentifier(), outWindow) == FALSE)
  {
    return FALSE;
  }

  AssetsListWindowData* data = (AssetsListWindowData*)windowGetInternalData(*outWindow);
  windowSetInternalData(*outWindow, data);
  
  return TRUE;
}

string assetsListWindowGetIdentifier()
{
  return "AssetsManagerList";
}

bool8 assetsListWindowInitialize(Window* window)
{
  return TRUE;
}

void assetsListWindowShutdown(Window* window)
{
  AssetsListWindowData* data = (AssetsListWindowData*)windowGetInternalData(window);
  if(data != nullptr)
  {
    engineFreeObject(data, MEMORY_TYPE_GENERAL);
  }
}

void assetsListWindowUpdate(Window* window, float64 delta)
{

}

void assetsListWindowDraw(Window* window, float64 delta)
{
  vector<Asset*> assetsToList = assetsManagerGetAssets();
  AssetsListWindowData* data = (AssetsListWindowData*)windowGetInternalData(window);
  
  bool8 isEmpty = assetsToList.size() == 0 ? TRUE : FALSE;
}

void assetsListWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{
  
}
