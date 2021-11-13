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

