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
#include "editor_utils.h"
#include "window_manager.h"
#include "assets_manager_window.h"
#include "material_settings_window.h"
#include "script_function_settings_window.h"

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
  const char* categoryName;
  const char* dataName;
  AssetPtr(*create)();
  void(*edit)(AssetPtr asset);
};

struct AssetCreationData
{
  const char* dataName;
  AssetPtr(*create)();
};

static bool8 assetsManagerWindowInitialize(Window* window);
static void assetsManagerWindowShutdown(Window* window);
static void assetsManagerWindowUpdate(Window* window, float64 delta);
static void assetsManagerWindowDraw(Window* window, float64 delta);
static void assetsManagerWindowProcessInput(Window* window, const EventData& eventData, void* sender);

static AssetPtr createGeometryAsset() { /** TODO */ }
static void editGeometryAsset(AssetPtr asset) { /** TODO */ }

static AssetPtr createSDFAsset();
static AssetPtr createIDFAsset();
static AssetPtr createODFAsset();
static AssetPtr createPCFAsset();
static void editScriptFunctionAsset(AssetPtr asset);

static AssetPtr createMaterialAsset();
static void editMaterialAsset(AssetPtr asset);

enum CategoryType
{
  CATEGORY_TYPE_GEOMETRY,
  CATEGORY_TYPE_MATERIAL,  
  CATEGORY_TYPE_SDF,
  CATEGORY_TYPE_IDF,
  CATEGORY_TYPE_ODF,
  CATEGORY_TYPE_PDF,

  CATEGORY_TYPE_COUNT
};

static vector<AssetsCategoryData> categoriesData =
{
  // Geometry category data
  {
    "Geometry",
    "geometry",
    createGeometryAsset,
    editGeometryAsset,
  },

  // Material category data
  {
    "Material",
    "material",
    createMaterialAsset,
    editMaterialAsset
  },

  // SDF category data
  {
    "SDF",
    "SDF",
    createSDFAsset,
    editScriptFunctionAsset
  },
  
  // IDF category data
  {
    "IDF",
    "IDF",
    createIDFAsset,
    editScriptFunctionAsset
  },
    
  // ODF category data
  {
    "ODF",
    "ODF",
    createODFAsset,
    editScriptFunctionAsset
  },

  // PCF category data
  {
    "PCF",
    "PCF",
    createPCFAsset,
    editScriptFunctionAsset
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
  
  if(ImGui::TreeNode(categoryData.dataName, "%s assets", categoryData.categoryName))
  {
    pushIconSmallButtonStyle();    

    char createButtonName[128];
    sprintf(createButtonName, "[Create new %s asset]", categoryData.dataName);

    ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);
      if(ImGui::SmallButton(createButtonName))
      {
        LOG_INFO("Create new %s", categoryData.dataName);

        char newAssetName[255];
        uint32 idx = 1;
        do
        {
          sprintf(newAssetName, "%s%d", categoryData.dataName, idx++);
        } while(assetsManagerHasAsset(newAssetName) == TRUE);

        AssetPtr newAsset = categoryData.create();
        assetSetName(newAsset, newAssetName);

        assetsManagerAddAsset(newAsset);
      }
    ImGui::PopStyleColor();
    
    uint32 idx = 1;
    for(AssetPtr asset: assets)
    {
      ImGui::Text("%3d. %-32s", idx, assetGetName(asset).c_str());
      ImGui::SameLine();

      ImGui::PushID(asset.raw());

        if(ImGui::Button(ICON_KI_PENCIL"##AssetRename"))
        {
          strcpy(textInputPopupGetBuffer(), assetGetName(asset).c_str());
          ImGui::OpenPopup(renamePopupName);
        }
        ImGui::SameLine();

        if(ImGui::IsPopupOpen(renamePopupName))
        {
          popIconSmallButtonStyle();
            ImGuiUtilsButtonsFlags button = textInputPopup(renamePopupName, "Enter new name");
          pushIconSmallButtonStyle();


          if(button == ImGuiUtilsButtonsFlags_Accept)
          {
            const char* newAssetName = textInputPopupGetBuffer();            
            if(assetsManagerHasAsset(newAssetName) == FALSE)
            {
              assetSetName(asset, newAssetName);
            }
            else
            {
              LOG_ERROR("Asset with name '%s' already exists!", newAssetName);
            }
          }
        }
          
        if(ImGui::Button(ICON_KI_COG"##AssetEdit"))
        {
          categoryData.edit(asset);
        }
        ImGui::SameLine();
        
        ImGui::PushStyleColor(ImGuiCol_Text, (float4)DeleteClr);
          if(ImGui::Button(ICON_KI_TRASH"##AssetDelete"))
          {
            assetsManagerRemoveAsset(asset.raw());
          }
        ImGui::PopStyleColor();

      ImGui::PopID();
      idx++;
    }
    
    popIconSmallButtonStyle();   
    ImGui::TreePop();
  }
}

void assetsManagerWindowDraw(Window* window, float64 delta)
{
  // TODO: Recalculate arrays only when assets list has changed
  vector<AssetPtr> assetsToList = assetsManagerGetAssets();  
  vector<AssetPtr> categoriesAssets[CATEGORY_TYPE_COUNT];
  
  for(AssetPtr asset: assetsToList)
  {
    AssetType type = assetGetType(asset);
    switch(type)
    {
      case ASSET_TYPE_GEOMETRY: categoriesAssets[CATEGORY_TYPE_GEOMETRY].push_back(asset); break;
      case ASSET_TYPE_MATERIAL: categoriesAssets[CATEGORY_TYPE_MATERIAL].push_back(asset); break;
      case ASSET_TYPE_SCRIPT_FUNCTION:
      {
        ScriptFunctionType sfType = scriptFunctionGetType(asset);
        switch(sfType)
        {
          case SCRIPT_FUNCTION_TYPE_SDF: categoriesAssets[CATEGORY_TYPE_SDF].push_back(asset); break;
          case SCRIPT_FUNCTION_TYPE_IDF: categoriesAssets[CATEGORY_TYPE_IDF].push_back(asset); break;
          case SCRIPT_FUNCTION_TYPE_ODF: categoriesAssets[CATEGORY_TYPE_ODF].push_back(asset); break;
          case SCRIPT_FUNCTION_TYPE_PCF: categoriesAssets[CATEGORY_TYPE_PDF].push_back(asset); break;
          default: assert(false);
        }
      } break;
      default: LOG_ERROR("Asset %p has unknown type %d", asset, (int)type); assert(false); break;
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

  for(uint32 cType = CATEGORY_TYPE_GEOMETRY; cType < CATEGORY_TYPE_COUNT; cType++)
  {
    drawAssetsCategory(window, categoriesData[cType], categoriesAssets[cType]);
  }

}

void assetsManagerWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{
  
}

AssetPtr createSDFAsset()
{
  return AssetPtr(scriptFunctionClone(createDefaultSDF()));
}

AssetPtr createIDFAsset()
{
  return AssetPtr(scriptFunctionClone(createDefaultIDF()));
}

AssetPtr createODFAsset()
{
  return AssetPtr(scriptFunctionClone(createDefaultODF()));
}

AssetPtr createPCFAsset()
{
  return AssetPtr(scriptFunctionClone(createDefaultPCF()));
}

void editScriptFunctionAsset(AssetPtr asset)
{
  WindowPtr openedWindow = windowManagerGetWindow(scriptFunctionWindowIdentifier(asset));
  if(openedWindow != nullptr)
  {
    windowSetFocused(openedWindow, TRUE);
  }
  else
  {
    Window* settingsWindow = nullptr;
    createScriptFunctionSettingsWindow(AssetPtr(nullptr), asset, &settingsWindow);
    windowSetSize(settingsWindow, float2(640.0f, 360.0f));
    
    windowManagerAddWindow(WindowPtr(settingsWindow));
  }
}

static AssetPtr createMaterialAsset()
{
  Asset* material;
  createMaterial("new_material", &material);

  return AssetPtr(material);
}

static void editMaterialAsset(AssetPtr asset)
{
  WindowPtr openedWindow = windowManagerGetWindow(materialSettingsWindowIdentifier(asset));
  if(openedWindow != nullptr)
  {
    windowSetFocused(openedWindow, TRUE);
  }
  else
  {
    Window* settingsWindow = nullptr;
    createMaterialSettingsWindow(asset, AssetPtr(nullptr), &settingsWindow);
    windowSetSize(settingsWindow, float2(640.0f, 360.0f));
    
    windowManagerAddWindow(WindowPtr(settingsWindow));
  }  
}
