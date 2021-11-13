#pragma once

#include <defines.h>

enum ImGuiUtilsButtonsFlags_
{
  ImGuiUtilsButtonsFlags_None   = 0,
  ImGuiUtilsButtonsFlags_Accept = 1 << 0,  
  ImGuiUtilsButtonsFlags_Cancel = 1 << 1,

  ImGuiUtilsButtonsFlags_Default = 3
};

typedef int ImGuiUtilsButtonsFlags;

void pushIconButtonStyle();
void popIconButtonStyle();

ImGuiUtilsButtonsFlags textInputPopup(const char* name,
                                      const char* tip,
                                      char* text,
                                      uint32 textSize,
                                      ImGuiUtilsButtonsFlags buttons = ImGuiUtilsButtonsFlags_Default);

