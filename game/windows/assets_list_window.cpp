#include <imgui/imgui.h>

#include <memory_manager.h>
#include <assets/assets_manager.h>

#include "assets_list_window.h"

struct AssetsListWindowData
{

  std::vector<Asset*> assets;
  fpOnAssetsListItemSelected selectCb;
  void* cbUserData;

  bool8 listAllAssets;
};

static bool8 assetsListWindowInitialize(Window* window);
static void assetsListWindowShutdown(Window* window);
static void assetsListWindowUpdate(Window* window, float64 delta);
static void assetsListWindowDraw(Window* window, float64 delta);
static void assetsListWindowProcessInput(Window* window, const EventData& eventData, void* sender);

bool8 createAssetsListWindowWithAllAssets(Window** outWindow)
{
  bool8 status = createAssetsListWindowWithSomeAssets(assetsManagerGetAssets(), outWindow);
  if(status == FALSE)
  {
    return FALSE;
  }

  AssetsListWindowData* data = (AssetsListWindowData*)windowGetInternalData(*outWindow);
  data->listAllAssets = TRUE;
  
  return TRUE;
}

bool8 createAssetsListWindowWithSomeAssets(const std::vector<Asset*>& assetsToList, Window** outWindow)
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

  AssetsListWindowData* data = engineAllocObject<AssetsListWindowData>(MEMORY_TYPE_GENERAL);
  data->assets = assetsToList;

  windowSetInternalData(*outWindow, data);
  
  return TRUE;
}

std::string assetsListWindowGetIdentifier()
{
  return "AssetsManagerList";
}

void assetsListWindowSetSelectCallback(Window* window, fpOnAssetsListItemSelected callback, void* userData)
{
  AssetsListWindowData* data = (AssetsListWindowData*)windowGetInternalData(window);
  data->selectCb = callback;
  data->cbUserData = userData;
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
  AssetsListWindowData* data = (AssetsListWindowData*)windowGetInternalData(window);
  if(data->listAllAssets == TRUE)
  {
    data->assets = assetsManagerGetAssets();
  }

  bool8 isEmpty = data->assets.size() == 0 ? TRUE : FALSE;
  bool8 isSelectionList = data->selectCb != nullptr ? TRUE : FALSE;

  if(isSelectionList == TRUE)
  {
    ImGui::Text("Select a desired asset");
    ImGui::Spacing();
  }
  
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
  uint32 assetIdx = 0;
  for(Asset* asset: data->assets)
  {
    if(ImGui::Selectable(assetGetName(asset).c_str(), -1))
    {
      if(isSelectionList == TRUE)
      {
        data->selectCb(window, asset, assetIdx, data->cbUserData);
        windowClose(window);
      }
    }

    ImGui::SameLine();
    ImGui::Text(ICON_KI_CARET_RIGHT);
  }
  ImGui::PopStyleVar();


  if(isSelectionList == TRUE)
  {
    ImGuiWindowFlags windowFlags = windowGetFlags(window);
    // NOTE: If it's a selection list - hide titlebar of the window    
    if((windowFlags & ImGuiWindowFlags_NoTitleBar) == 0)
    {
      windowFlags |= ImGuiWindowFlags_NoTitleBar;
      windowSetFlags(window, windowFlags);
    }

    if((windowFlags & ImGuiWindowFlags_NoResize) == 0)
    {
      windowFlags |= ImGuiWindowFlags_NoResize;
      windowSetFlags(window, windowFlags);
    }

    // NOTE: Close on lose focus
    if(windowIsFocused(window) == FALSE)
    {
      windowClose(window);
    }
    
    windowSetStyle(window, ImGuiStyleVar_WindowRounding, 0.0f);
    windowSetStyle(window, ImGuiStyleVar_WindowBorderSize, 0.5f);    
  }
}

void assetsListWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{
  
}
