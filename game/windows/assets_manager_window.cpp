#include <unordered_map>

#include <imgui/imgui.h>

#include <utils.h>
#include <logging.h>
#include <memory_manager.h>

#include <assets/geometry.h>
#include <assets/material.h>
#include <assets/assets_manager.h>
#include <assets/script_function.h>

#include "ui_utils.h"
#include "ui_styles.h"
#include "assets_manager_window.h"

#include <ptr.h>

using std::vector;
using std::string;
using std::unordered_map;

struct AssetsListWindowData
{
  char pass;
};

struct AssetsCategoryData
{
  AssetType categoryAssetType;
  const char* dataName;
  Asset*(*create)();
  void(*edit)();
};

static bool8 assetsManagerWindowInitialize(Window* window);
static void assetsManagerWindowShutdown(Window* window);
static void assetsManagerWindowUpdate(Window* window, float64 delta);
static void assetsManagerWindowDraw(Window* window, float64 delta);
static void assetsManagerWindowProcessInput(Window* window, const EventData& eventData, void* sender);

static Asset* createGeometryAsset() { /** TODO */ }
static void editGeometryAsset() { /** TODO */ }

static Asset* createScriptFunctionAsset();
static void editScriptFunctionAsset();

static Asset* createMaterialAsset() { /** TODO */ }
static void editMaterialAsset() { /** TODO */ }

static unordered_map<AssetType, AssetsCategoryData> categoryData =
{
  // Geometry category data
  {
    ASSET_TYPE_GEOMETRY,
    {
      ASSET_TYPE_GEOMETRY,      
      "Geometry",
      createGeometryAsset,
      editGeometryAsset,
    }
  },
  
  // Script function category data
  {
    ASSET_TYPE_SCRIPT_FUNCTION,
    {
      ASSET_TYPE_SCRIPT_FUNCTION,    
      "Script function",
      createScriptFunctionAsset,
      editScriptFunctionAsset
    }
  },

  // Material category data
  {
    ASSET_TYPE_MATERIAL,
    {
      ASSET_TYPE_MATERIAL,    
      "Material",
      createMaterialAsset,
      editMaterialAsset
    }
  }
};

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

static void drawAssetsCategory(Window* window, const AssetsCategoryData& categoryData, vector<AssetPtr> assets)
{
  static const char* renamePopupName = "Asset rename popup";
  
  if(ImGui::TreeNode(categoryData.dataName, "%s assets", categoryData.dataName))
  {
    pushIconButtonStyle();    

    char createButtonName[128];
    sprintf(createButtonName, "[Create new %s asset]", categoryData.dataName);

    ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);
      if(ImGui::SmallButton(createButtonName))
      {
        LOG_INFO("Create new %s", categoryData.dataName);
      }
    ImGui::PopStyleColor();
    
    uint32 idx = 1;
    for(AssetPtr asset: assets)
    {
      ImGui::Text("%3d. %-32s", idx, assetGetName(asset).c_str());
      ImGui::SameLine();

      ImGui::PushID(asset.ptr);

        if(ImGui::Button(ICON_KI_PENCIL"##AssetRename"))
        {
          strcpy(textInputPopupGetBuffer(), assetGetName(asset).c_str());
          ImGui::OpenPopup(renamePopupName);
        }
        ImGui::SameLine();

        if(ImGui::IsPopupOpen(renamePopupName))
        {
          popIconButtonStyle();
            ImGuiUtilsButtonsFlags button = textInputPopup(renamePopupName, "Enter new name");
          pushIconButtonStyle();
          
          if(button == ImGuiUtilsButtonsFlags_Accept)
          {
            assetSetName(asset, textInputPopupGetBuffer());
          }
        }
          
        ImGui::Button(ICON_KI_COG"##AssetEdit");
        ImGui::SameLine();
        
        ImGui::PushStyleColor(ImGuiCol_Text, (float4)DeleteClr);
          if(ImGui::Button(ICON_KI_TRASH"##AssetDelete"))
          {
            assetsManagerRemoveAsset(asset.ptr);
          }
        ImGui::PopStyleColor();

      ImGui::PopID();
      idx++;
    }
    
    popIconButtonStyle();   
    ImGui::TreePop();
  }
}

void assetsManagerWindowDraw(Window* window, float64 delta)
{
  // TODO: Recalculate arrays only when assets list has changed
  vector<AssetPtr> assetsToList = assetsManagerGetAssets();  
  vector<AssetPtr> geometryAssets;
  vector<AssetPtr> scriptFunctionAssets;
  vector<AssetPtr> materialAssets;
  
  for(AssetPtr asset: assetsToList)
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

  static char searchInputBuffer[255];
  ImGui::InputTextWithHint("##SearchAssetInput",
                           ICON_KI_SEARCH" Enter name of asset",
                           searchInputBuffer,
                           ARRAY_SIZE(searchInputBuffer));

  ImGui::SameLine();
  if(ImGui::Button("Search##AssetsManager"))
  {
    
  }
  
  ImGui::SameLine();
  if(ImGui::Button("Filter##AssetsManager"))
  {
    ImGui::OpenPopup("Filter popup##AssetsManager");
  }
  
  drawAssetsCategory(window, categoryData[ASSET_TYPE_GEOMETRY], geometryAssets);
  drawAssetsCategory(window, categoryData[ASSET_TYPE_SCRIPT_FUNCTION], scriptFunctionAssets);
  drawAssetsCategory(window, categoryData[ASSET_TYPE_MATERIAL], materialAssets);
}

void assetsManagerWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{
  
}

Asset* createScriptFunctionAsset()
{
  Asset* newSF = nullptr;
  assert(createScriptFunction(SCRIPT_FUNCTION_TYPE_SDF, "", &newSF));

  return newSF;
}

void editScriptFunctionAsset()
{

}
