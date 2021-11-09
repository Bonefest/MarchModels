#include <imgui/imgui.h>

#include <logging.h>
#include <memory_manager.h>

#include <assets/geometry.h>
#include <assets/material.h>
#include <assets/assets_manager.h>
#include <assets/script_function.h>

#include "assets_manager_window.h"

using std::vector;
using std::string;

struct AssetsListWindowData
{
  char pass;
};

static bool8 assetsManagerWindowInitialize(Window* window);
static void assetsManagerWindowShutdown(Window* window);
static void assetsManagerWindowUpdate(Window* window, float64 delta);
static void assetsManagerWindowDraw(Window* window, float64 delta);
static void assetsManagerWindowProcessInput(Window* window, const EventData& eventData, void* sender);

bool8 createAssetsManagerWindow(Window** outWindow)
{
  WindowInterface interface = {};
  interface.initialize = assetsManagerWindowInitialize;
  interface.shutdown = assetsManagerWindowShutdown;
  interface.update = assetsManagerWindowUpdate;
  interface.draw = assetsManagerWindowDraw;
  interface.processInput = assetsManagerWindowProcessInput;

  if(allocateWindow(interface, assetsManagerWindowGetIdentifier(), outWindow) == FALSE)
  {
    return FALSE;
  }

  AssetsListWindowData* data = (AssetsListWindowData*)windowGetInternalData(*outWindow);
  windowSetInternalData(*outWindow, data);
  
  return TRUE;
}

string assetsManagerWindowGetIdentifier()
{
  return "AssetsManagerList";
}

bool8 assetsManagerWindowInitialize(Window* window)
{
  return TRUE;
}

void assetsManagerWindowShutdown(Window* window)
{
  AssetsListWindowData* data = (AssetsListWindowData*)windowGetInternalData(window);
  if(data != nullptr)
  {
    engineFreeObject(data, MEMORY_TYPE_GENERAL);
  }
}

void assetsManagerWindowUpdate(Window* window, float64 delta)
{

}

static void drawAssetsCategory(Window* window, const char* categoryName, vector<Asset*> assets)
{
  if(ImGui::TreeNode(categoryName))
  {
    uint32 idx = 1;
    for(Asset* asset: assets)
    {
      ImGui::Text("%3d. %s", idx, assetGetName(asset).c_str());
      ImGui::SameLine();
      ImGui::Button("Edit");
      ImGui::SameLine();
      ImGui::Button("Rename");
      ImGui::SameLine();
      ImGui::Button("Delete");

      idx++;
    }
    
    ImGui::TreePop();
  }
}

void assetsManagerWindowDraw(Window* window, float64 delta)
{
  // TODO: Recalculate arrays only when assets list has changed
  vector<Asset*> assetsToList = assetsManagerGetAssets();  
  vector<Asset*> geometryAssets;
  vector<Asset*> scriptFunctionAssets;
  vector<Asset*> materialAssets;
  
  for(Asset* asset: assetsToList)
  {
    AssetType type = assetGetType(asset);
    if(type == ASSET_TYPE_GEOMETRY)
    {
      geometryAssets.push_back(asset);
    }
    else if(type == ASSET_TYPE_SCRIPT_FUNCTION)
    {
      scriptFunctionAssets.push_back(asset);
    }
    else if(type == ASSET_TYPE_MATERIAL)
    {
      materialAssets.push_back(asset);
    }
    else
    {
      LOG_ERROR("Asset %p has unknown type %d", asset, (int)type);
    }
  }
  
  AssetsListWindowData* data = (AssetsListWindowData*)windowGetInternalData(window);
  
  bool8 isEmpty = assetsToList.size() == 0 ? TRUE : FALSE;

  char temp[255];
  ImGui::InputTextWithHint("##SearchAssetInput", "Enter name of asset", temp, 255);
  ImGui::SameLine();
  ImGui::Button("Filter##AssetsManager");

  drawAssetsCategory(window, "Geometry assets", geometryAssets);
  drawAssetsCategory(window, "Script functions assets", scriptFunctionAssets);
  drawAssetsCategory(window, "Material assets", materialAssets);
}

void assetsManagerWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{
  
}
