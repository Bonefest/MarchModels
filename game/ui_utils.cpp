#include <imgui/imgui.h>

#include <utils.h>
#include <assets/geometry.h>
#include <assets/material.h>
#include <assets/assets_manager.h>
#include <assets/script_function.h>

#include "ui_utils.h"
#include "ui_styles.h"
#include "windows/list_window.h"
#include "windows/window_manager.h"
#include "windows/light_settings_window.h"
#include "windows/material_settings_window.h"
#include "windows/geometry_settings_window.h"
#include "windows/script_function_settings_window.h"

static const uint32 textInputPopupBufSize = 1024;
static char textInputPopupBuf[textInputPopupBufSize];

static void drawAssetsList(const std::string& listName,
                           const std::string& assetName,
                           const std::vector<ListItem>& items,
                           fpOnListItemSelected selectedCallback,
                           void* userData = nullptr)
{
  float2 itemTopPos = ImGui::GetItemRectMin();

  WindowPtr prevListWindow = windowManagerGetWindow(listName);
  if(prevListWindow != nullptr)
  {
    windowManagerRemoveWindow(prevListWindow);
  }

  char title[128];
  sprintf(title, "Select a %s", assetName.c_str());
  
  Window* assetsListWindow;
  assert(createListWindow(listName,
                          title,
                          items,
                          selectedCallback,
                          userData,
                          &assetsListWindow));

  listWindowSetCloseOnLoseFocus(assetsListWindow, TRUE);
  windowSetSize(assetsListWindow, float2(180.0, 100.0));
  windowSetPosition(assetsListWindow, itemTopPos + float2(10, 10));
  windowSetFocused(assetsListWindow, TRUE);

  windowManagerAddWindow(WindowPtr(assetsListWindow));        
}

void pushIconSmallButtonStyle()
{
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, float2(0.0f, 0.0f));
  pushIconButtonStyle();
}

void popIconSmallButtonStyle()
{
  ImGui::PopStyleVar();
  popIconButtonStyle();
}

void pushIconButtonStyle()
{
  ImGui::PushStyleColor(ImGuiCol_Button, (float4)ImColor(0, 0, 0, 0));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (float4)ImColor(0, 0, 0, 0));    
}

void popIconButtonStyle()
{
  ImGui::PopStyleColor(2);
}

char* textInputPopupGetBuffer()
{
  return textInputPopupBuf;
}

ImGuiUtilsButtonsFlags textInputPopup(const char* name,
                                      const char* tip,
                                      ImGuiUtilsButtonsFlags buttons)
{
  return textInputPopupCustom(name, tip, textInputPopupBuf, textInputPopupBufSize, buttons);
}


ImGuiUtilsButtonsFlags textInputPopupCustom(const char* name,
                                            const char* tip,
                                            char* text,
                                            uint32 textSize,
                                            ImGuiUtilsButtonsFlags buttons)
{
  const float32 popupWidth = 200.0f;
  ImGuiUtilsButtonsFlags pressedButton = ImGuiUtilsButtonsFlags_None;
  
  ImGuiStyle& style = ImGui::GetStyle();
  ImGui::SetNextWindowSize(float2(popupWidth, 0.0f));
  if(ImGui::BeginPopupModal(name, NULL, ImGuiWindowFlags_NoResize))
  {
    ImGui::SetNextItemWidth(0.0f);

    if(strlen(tip) > 0)
    {
      ImGui::Text("Enter a new name: ");
    }


    ImGui::SetNextItemWidth(ImGui::GetWindowInnerAreaSize().x);
    ImGui::InputText("##NewNameInput", text, textSize);

    bool8 acceptBut = (buttons & ImGuiUtilsButtonsFlags_Accept) == ImGuiUtilsButtonsFlags_Accept ? TRUE : FALSE;
    bool8 cancelBut = (buttons & ImGuiUtilsButtonsFlags_Cancel) == ImGuiUtilsButtonsFlags_Cancel ? TRUE : FALSE;
    float32 butWidth = ImGui::GetWindowInnerAreaSize().x;
    if(acceptBut == TRUE && cancelBut == TRUE)
    {
      butWidth = (butWidth - style.ItemSpacing.x) * 0.5f;
    }

    if(acceptBut == TRUE)
    {
      if(ImGui::Button("Accept", float2(butWidth, 0)))
      {
        ImGui::CloseCurrentPopup();
        pressedButton = ImGuiUtilsButtonsFlags_Accept;
      }
    }

    if(cancelBut == TRUE)
    {
      if(acceptBut == TRUE)
      {
        ImGui::SameLine();
      }

      if(ImGui::Button("Cancel", float2(butWidth, 0)))
      {
        pressedButton = ImGuiUtilsButtonsFlags_Cancel;
        ImGui::CloseCurrentPopup();
      }
    }
      
    ImGui::EndPopup();
  }

  return pressedButton;
}

void openScriptFunctionSettingsWindow(AssetPtr geometryOwner, AssetPtr function)
{
  WindowPtr scriptFunctionSettingsWindow = windowManagerGetWindow(scriptFunctionWindowIdentifier(function));
  if(scriptFunctionSettingsWindow == nullptr)
  {
    Window* newSettingsWindow = nullptr;
    assert(createScriptFunctionSettingsWindow(geometryOwner, function, &newSettingsWindow));
    windowSetSize(newSettingsWindow, float2(640.0f, 360.0f));
    windowManagerAddWindow(WindowPtr(newSettingsWindow));
  }
  else
  {
    windowSetFocused(scriptFunctionSettingsWindow, TRUE);
  }
}

void drawMaterialItem(AssetPtr geometry, AssetPtr material)
{
  ImGui::TextColored("_<C>%#010x</C>_[MAT] _<C>0x1</C>_'%s'",
                     revbytes((uint32)WarningClr),
                     assetGetName(material).c_str());

  ImGui::SameLine();

  pushIconSmallButtonStyle();
  
  if(ImGui::SmallButton(ICON_KI_LIST))
  {
    std::vector<AssetPtr> assetsToDisplay = assetsManagerGetAssetsByType(ASSET_TYPE_MATERIAL);

    std::vector<ListItem> items = {};
    for(AssetPtr asset: assetsToDisplay)
    {
      items.push_back(ListItem{assetGetName(asset), asset});
    }

    auto onAssetIsSelected = [geometry](Window* window, void* selection, uint32 index, void* target)
    {
      AssetPtr material = assetsManagerFindAsset(assetGetName((Asset*)selection));      
      geometrySetMaterial((Asset*)target, material);
    };

    drawAssetsList("Materials list", "material", items, onAssetIsSelected, geometry.raw());    
  }

  ImGui::SameLine();
  
  if(ImGui::SmallButton(ICON_KI_COG))
  {
    WindowPtr openedWindow = windowManagerGetWindow(materialSettingsWindowIdentifier(material));
    if(openedWindow != nullptr)
    {
      windowSetFocused(openedWindow, TRUE);
    }
    else
    {
      Window* settingsWindow = nullptr;
      createMaterialSettingsWindow(material, geometry, &settingsWindow);
      windowSetSize(settingsWindow, float2(640.0f, 360.0f));

      windowManagerAddWindow(WindowPtr(settingsWindow));
    }  
  }
  
  popIconSmallButtonStyle();
}

void drawScriptFunctionItem(AssetPtr geometry, AssetPtr function, uint32 index)
{
  ScriptFunctionType type = scriptFunctionGetType(function);  
  bool8 isLeaf = geometryIsLeaf(geometry);
  
  pushIconSmallButtonStyle();
  ImGui::PushID(index);
  ImGui::BeginDisabled((isLeaf == TRUE && type == SCRIPT_FUNCTION_TYPE_PCF) ||
                       (isLeaf == FALSE && type == SCRIPT_FUNCTION_TYPE_SDF));
                       

    const char* functionTypeLabel = scriptFunctionTypeLabel(type);

    ImGui::TextColored("_<C>%#010x</C>_[%s] _<C>0x1</C>_'%s'",
                       revbytes((uint32)SuccessClr),
                       functionTypeLabel,
                       assetGetName(function).c_str());

    ImGui::SameLine();

    // NOTE: Script functions list button
    if(ImGui::SmallButton(ICON_KI_LIST))
    {
      std::vector<AssetPtr> assetsToDisplay = assetsManagerGetAssetsByType(ASSET_TYPE_SCRIPT_FUNCTION);

      // NOTE: Remove all script functions that don't have similar type
      auto removeIt = std::remove_if(assetsToDisplay.begin(),
                                     assetsToDisplay.end(),
                                     [function](AssetPtr asset) {
                                       return scriptFunctionGetType(asset) != scriptFunctionGetType(function);
                                     });

      assetsToDisplay.erase(removeIt, assetsToDisplay.end());

      std::vector<ListItem> items = {};
      for(AssetPtr asset: assetsToDisplay)
      {
        items.push_back(ListItem{assetGetName(asset), asset});
      }

      auto onScriptFunctionIsSelected = [geometry](Window* window, void* selection, uint32 index, void* target)
      {
        AssetPtr assetFromManager = assetsManagerFindAsset(assetGetName((Asset*)selection));
        if(assetFromManager != nullptr)
        {
          geometryRemoveFunction(geometry, (Asset*)target);
          geometryAddFunction(geometry, assetFromManager);
        }
      };

      drawAssetsList("Script functions list", "function", items, onScriptFunctionIsSelected, function);
    }

    ImGui::SameLine();

    // NOTE: Script function settings button
    if(ImGui::SmallButton(ICON_KI_COG))
    {
      openScriptFunctionSettingsWindow(geometry, function);
    }

    ImGui::SameLine();

  // NOTE: We do not want to disable removing button, so we end disabling here
  ImGui::EndDisabled();

  // NOTE: PCF and SDF functions cannot be removed
  ImGui::BeginDisabled(type == SCRIPT_FUNCTION_TYPE_PCF || type == SCRIPT_FUNCTION_TYPE_SDF);
    // NOTE: Script function removing button
    ImGui::PushStyleColor(ImGuiCol_Text, (float4)DeleteClr);
      if(ImGui::SmallButton(ICON_KI_TRASH))
      {
        // If settings window is opened - close it manually
        if(windowManagerHasWindow(scriptFunctionWindowIdentifier(function)) == TRUE)
        {
          Window* scriptFunctionSettingsWindow = windowManagerGetWindow(scriptFunctionWindowIdentifier(function));
          windowClose(scriptFunctionSettingsWindow);
        }

        geometryRemoveFunction(geometry, function);
      }
    ImGui::PopStyleColor();
  ImGui::EndDisabled();

  ImGui::PopID();
  popIconSmallButtonStyle();
}

bool8 drawGeometryItemActionButtons(Scene* scene, AssetPtr geometry)
{
  bool8 removeIsPressed = FALSE;

  ImGui::PushID(geometry);
  pushIconSmallButtonStyle();
    if(ImGui::SmallButton(ICON_KI_PENCIL"##GeometryChangeName"))
    {
      ImGui::OpenPopup("Change geometry name");
      strcpy(textInputPopupGetBuffer(), assetGetName(geometry).c_str());
    }

  popIconSmallButtonStyle();
      ImGuiUtilsButtonsFlags pressedButton = textInputPopup("Change geometry name", "Enter a new name");

      if(ImGuiUtilsButtonsFlags_Accept == pressedButton)
      {
        assetSetName(geometry, textInputPopupGetBuffer());
      }
      
  pushIconSmallButtonStyle();
    ImGui::SameLine();       
    if(ImGui::SmallButton(ICON_KI_LIST"##GeometryChoose"))
    {
      
      drawGeometriesList(geometry);
    }
    
    ImGui::SameLine();
    if(ImGui::SmallButton(ICON_KI_COG"##GeometryEdit"))
    {
      WindowPtr geometrySettingsWindow = windowManagerGetWindow(geometrySettingsWindowIdentifier(geometry));
      if(geometrySettingsWindow == nullptr)
      {
        Window* newSettingsWindow = nullptr;        
        assert(createGeometrySettingsWindow(scene, geometry, &newSettingsWindow));
        windowSetSize(newSettingsWindow, float2(480.0f, 180.0f));
        windowManagerAddWindow(WindowPtr(newSettingsWindow));
      }
      else
      {
        windowSetFocused(geometrySettingsWindow, TRUE);
      }      
    }
    
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, (float4)DeleteClr);
      if(ImGui::SmallButton(ICON_KI_TRASH"##GeometryRemove"))
      {
        removeIsPressed = TRUE;
      }
    ImGui::PopStyleColor();

  popIconSmallButtonStyle();
  ImGui::PopID();
  
  return removeIsPressed;
}

bool8 drawLightSourceItemActionButtons(AssetPtr lightSource)
{
  bool8 removeIsPressed = FALSE;

  pushIconSmallButtonStyle();
    if(ImGui::SmallButton(ICON_KI_PENCIL"##LightSourceChangeName"))
    {
      ImGui::OpenPopup("Change light source name");
      strcpy(textInputPopupGetBuffer(), assetGetName(lightSource).c_str());
    }
  popIconSmallButtonStyle();
  
  ImGuiUtilsButtonsFlags pressedButton = textInputPopup("Change light source name", "Enter a new name");

  if(ImGuiUtilsButtonsFlags_Accept == pressedButton)
  {
    assetSetName(lightSource, textInputPopupGetBuffer());
  }
      
  pushIconSmallButtonStyle();
  
    ImGui::SameLine();
    if(ImGui::SmallButton(ICON_KI_COG"##LightSourceEdit"))
    {
      WindowPtr lightSettingsWindow = windowManagerGetWindow(geometrySettingsWindowIdentifier(lightSource));
      if(lightSettingsWindow == nullptr)
      {
        Window* newSettingsWindow = nullptr;        
        assert(createLightSettingsWindow(lightSource, &newSettingsWindow));
        windowSetSize(newSettingsWindow, float2(480.0f, 180.0f));
        windowManagerAddWindow(WindowPtr(newSettingsWindow));
      }
      else
      {
        windowSetFocused(lightSettingsWindow, TRUE);
      }            
    }
    
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, (float4)DeleteClr);
      if(ImGui::SmallButton(ICON_KI_TRASH"##LightSourceRemove"))
      {
        removeIsPressed = TRUE;
      }
    ImGui::PopStyleColor();

  popIconSmallButtonStyle();
  
  return removeIsPressed;
}

void drawGeometriesList(AssetPtr geometry)
{
  std::vector<AssetPtr> assetsToDisplay = assetsManagerGetAssetsByType(ASSET_TYPE_GEOMETRY);

  std::vector<ListItem> items = {};
  for(AssetPtr asset: assetsToDisplay)
  {
    items.push_back(ListItem{assetGetName(asset), asset});
  }
  
  auto onAssetIsSelected = [geometry](Window* window, void* selection, uint32 index, void* target)
  {
    AssetPtr assetFromManager = assetsManagerFindAsset(assetGetName((Asset*)selection));
    if(assetFromManager != nullptr)
    {
      geometryCopy(geometry, (Asset*)selection);
    }
  };
  
  drawAssetsList("Geometries list", "geometry", items, onAssetIsSelected);
}
