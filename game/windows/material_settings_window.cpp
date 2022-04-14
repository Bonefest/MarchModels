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
  float4 uvRect = calculateUVRect(imageGetSize(defaultImage), uint2(832, 128), uint2(64, 64));  

  for(uint32 type = 0; type < MATERIAL_TEXTURE_TYPE_COUNT; type++)
  {
    ImagePtr image = materialGetTexture(data->material, (MaterialTextureType)type);
    
    char inputLabel[256];
    sprintf(inputLabel, "%s path", materialTextureTypeLabel((MaterialTextureType)type));
    
    if(image != ImagePtr(nullptr))
    {
      // TODO: Use only rect specified by user
      ImGui::Image((void*)imageGetGLHandle(image), ImVec2(32, 32));
    }
    else
    {
      ImGui::Image((void*)imageGetGLHandle(defaultImage), ImVec2(32, 32),
                   float2(uvRect.x, uvRect.y), float2(uvRect.z, uvRect.w),
                   float4(1.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
      
    }

    ImGui::SameLine();

    ImGui::PushItemWidth(128.0f);
      ImGui::InputText(inputLabel, data->texturePaths[type], 256);
    ImGui::PopItemWidth();
  }

}

void materialSettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{

}
