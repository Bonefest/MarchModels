#include <imgui/imgui.h>

#include <image_manager.h>
#include <assets/material.h>
#include <assets/assets_manager.h>

#include "utils.h"
#include "ui_utils.h"
#include "ui_styles.h"
#include "editor_utils.h"
#include "material_settings_window.h"

using std::string;

struct MaterialSettingsWindowData
{
  AssetPtr owner;
  AssetPtr material;

  char texturePaths[MATERIAL_TEXTURE_TYPE_COUNT][256];
};

static bool8 materialSettingsWindowInitialize(Window*);
static void materialSettingsWindowShutdown(Window*);
static void materialSettingsWindowUpdate(Window* window, float64 delta);
static void materialSettingsWindowDraw(Window* window, float64 delta);
static void materialSettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender);

bool8 createMaterialSettingsWindow(AssetPtr material, AssetPtr geometryOwner, Window** outWindow)
{
  WindowInterface interface = {};
  interface.initialize = materialSettingsWindowInitialize;
  interface.shutdown = materialSettingsWindowShutdown;
  interface.update = materialSettingsWindowUpdate;
  interface.draw = materialSettingsWindowDraw;
  interface.processInput = materialSettingsWindowProcessInput;

  if(allocateWindow(interface, materialSettingsWindowIdentifier(material), outWindow) == FALSE)
  {
    return FALSE;
  }

  MaterialSettingsWindowData* data = engineAllocObject<MaterialSettingsWindowData>(MEMORY_TYPE_GENERAL);
  data->owner = geometryOwner;
  data->material = material;

  windowSetInternalData(*outWindow, data);
  
  return TRUE;
}

string materialSettingsWindowIdentifier(Asset* material)
{
  char identifier[255];
  sprintf(identifier, "%s settings##%p", assetGetName(material).c_str(), material);

  return identifier;
}

bool8 materialSettingsWindowInitialize(Window* window)
{
  return TRUE;
}

void materialSettingsWindowShutdown(Window* window)
{
  MaterialSettingsWindowData* data = (MaterialSettingsWindowData*)windowGetInternalData(window);
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

void materialSettingsWindowUpdate(Window* window, float64 delta)
{

}

void materialSettingsWindowDraw(Window* window, float64 delta)
{
  MaterialSettingsWindowData* data = (MaterialSettingsWindowData*)windowGetInternalData(window);
  ImagePtr defaultImage = imageManagerLoadImage("assets/lights_sprites.png");
  float4 defaultUVRect = calculateUVRect(imageGetSize(defaultImage), uint2(832, 128), uint2(64, 64));
  float4 cancelUVRect = calculateUVRect(imageGetSize(defaultImage), uint2(832, 64), uint2(64, 64));

  for(uint32 type = 0; type < MATERIAL_TEXTURE_TYPE_COUNT; type++)
  {
    ImagePtr image = materialGetTexture(data->material, (MaterialTextureType)type);
    float32 imageSize = 48.0f;

    float2 imageStart = ImGui::GetCursorPos();

    ImagePtr imageToUse = image;
    float2 uvMin = float2(0.0f, 0.0f);
    float2 uvMax = float2(1.0f, 1.0f);

    // TODO: Else Use only rect specified by user    
    if(image == ImagePtr(nullptr))
    {
      uvMin = float2(defaultUVRect.x, defaultUVRect.y);
      uvMax = float2(defaultUVRect.z, defaultUVRect.w);

      imageToUse = defaultImage;
    }

    ImGui::Image((void*)imageGetGLHandle(imageToUse), ImVec2(imageSize, imageSize), uvMin, uvMax);

    ImGui::SetCursorPos(imageStart);

    if(ImGui::InvisibleButton("Disable", float2(imageSize, imageSize)))
    {
      LOG_INFO("Click");
    }

    ImGui::SetCursorPos(imageStart);
    
    if(TRUE) // TODO: If disabled
    {
      ImGui::Image((void*)imageGetGLHandle(defaultImage), ImVec2(imageSize, imageSize),
                   float2(cancelUVRect.x, cancelUVRect.y), float2(cancelUVRect.z, cancelUVRect.w));
    }
    
    ImGui::SameLine();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, float2(0.0f, 4.0f));
        ImGui::PushItemWidth(256.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, float4(0.2f, 0.2f, 0.2f, 0.54f));
          ImGui::InputTextWithHint("", "path to a file", data->texturePaths[type], 256);
        ImGui::PopStyleColor();
        ImGui::PopItemWidth();

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, (float4)SecondaryClr * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (float4)BrightSecondaryClr * 0.6f);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (float4)BrightSecondaryClr * 0.65f);            
          ImGui::Button("Load");
        ImGui::PopStyleColor(3);

      ImGui::PopStyleVar();
      ImGui::NewLine();
    
      ImGui::SetCursorPosY(imageStart.y + imageSize * 0.5f);
      ImGui::Dummy(float2(imageSize, 0.0f));
      ImGui::SameLine();

      int4 temp;
      ImGui::PushItemWidth(256.0f);
      ImGui::PushStyleColor(ImGuiCol_FrameBg, float4(0.13f, 0.13f, 0.13f, 0.54f));
        ImGui::InputInt4("Rect", &temp[0]);
      ImGui::PopStyleColor();
      ImGui::PopItemWidth();

      ImGui::NewLine();

      ImGui::SetCursorPos(imageStart);
      ImGui::Dummy(float2(imageSize, imageSize));

      ImGui::Text("%s", materialTextureTypeLabel((MaterialTextureType)type));
      ImGui::NewLine();
    ImGui::PopStyleVar();
  }

}

void materialSettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{

}
