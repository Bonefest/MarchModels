#include <imgui/imgui.h>

#include "ui_utils.h"

ImGuiUtilsButtonsFlags textInputPopup(const char* name,
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

