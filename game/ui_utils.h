#pragma once

#include <defines.h>

#include <scene.h>
#include <assets/asset.h>

enum ImGuiUtilsButtonsFlags_
{
  ImGuiUtilsButtonsFlags_None   = 0,
  ImGuiUtilsButtonsFlags_Accept = 1 << 0,  
  ImGuiUtilsButtonsFlags_Cancel = 1 << 1,

  ImGuiUtilsButtonsFlags_Default = 3
};

typedef int ImGuiUtilsButtonsFlags;

void pushIconSmallButtonStyle();
void popIconSmallButtonStyle();

void pushIconButtonStyle();
void popIconButtonStyle();

char* textInputPopupGetBuffer();

// NOTE: Display text input popup using a common global buffer
ImGuiUtilsButtonsFlags textInputPopup(const char* name,
                                      const char* tip,
                                      ImGuiUtilsButtonsFlags buttons = ImGuiUtilsButtonsFlags_Default);

// NOTE: Display text input pupup using a given custom buffer
ImGuiUtilsButtonsFlags textInputPopupCustom(const char* name,
                                            const char* tip,
                                            char* text,
                                            uint32 textSize,
                                            ImGuiUtilsButtonsFlags buttons = ImGuiUtilsButtonsFlags_Default);

void openScriptFunctionSettingsWindow(AssetPtr geometryOwner, AssetPtr function);

void drawScriptFunctionItem(AssetPtr geometry, AssetPtr function);
bool8 drawGeometryItemActionButtons(Scene* scene, AssetPtr geometry);


