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

  for(uint32 itype = 0; itype < MATERIAL_TEXTURE_TYPE_COUNT; itype++)
  {
    ImagePtr image = materialGetTexture(data->material, (MaterialTextureType)itype);
    if(image != ImagePtr(nullptr))
    {
      strcpy(data->texturePaths[itype], imageGetName(image).c_str());
    }
  }

  windowSetInternalData(*outWindow, data);
  windowSetStyle(*outWindow, ImGuiStyleVar_WindowPadding, float2(12.0f, 12.0f));  
  
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


#include "assets/materials_atlas_system.h"
void materialSettingsWindowDraw(Window* window, float64 delta)
{
  MaterialSettingsWindowData* data = (MaterialSettingsWindowData*)windowGetInternalData(window);
  ImagePtr defaultImage = imageManagerLoadImage("assets/lights_sprites.png");
  float4 defaultUVRect = calculateUVRect(imageGetSize(defaultImage), uint2(832, 128), uint2(64, 64));
  float4 cancelUVRect = calculateUVRect(imageGetSize(defaultImage), uint2(832, 64), uint2(64, 64));

  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
  ImGui::PushStyleColor(ImGuiCol_FrameBg, float4(0.2f, 0.2f, 0.2f, 0.54f));
  ImGui::PushStyleColor(ImGuiCol_Button, (float4)SecondaryClr * 0.5f);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (float4)BrightSecondaryClr * 0.6f);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, (float4)BrightSecondaryClr * 0.65f);  
  ImGui::PushStyleColor(ImGuiCol_SliderGrab, float4(0.41f, 0.41f, 0.41f, 1.0f));
      
  ImGui::Text("Texture settings");
  ImGui::Separator();
  
  // --- Projection mode ------------------------------------------------------
  static const char* projectionModeLabels[] =
  {
    materialTextureProjectionModeLabel(MATERIAL_TEXTURE_PROJECTION_MODE_TRIPLANAR),
    materialTextureProjectionModeLabel(MATERIAL_TEXTURE_PROJECTION_MODE_SPHERICAL),
    materialTextureProjectionModeLabel(MATERIAL_TEXTURE_PROJECTION_MODE_CYLINDRICAL),    
  };
  
  MaterialTextureProjectionMode projectionMode = materialGetProjectionMode(data->material);
  
  if(ImGui::Combo("Projection mode", (int32*)&projectionMode, projectionModeLabels,
                  ARRAY_SIZE(projectionModeLabels)))
  {
    materialSetProjectionMode(data->material, projectionMode);
  }
  
  // --- Texture choosing -----------------------------------------------------

  for(uint32 itype = 0; itype < MATERIAL_TEXTURE_TYPE_COUNT; itype++)
  {
    const float32 imageSize = 64.0f;
    
    ImGui::PushID(itype);

    MaterialTextureType type = (MaterialTextureType)itype;
    
    ImagePtr image = materialGetTexture(data->material, type);

    float2 imageStart = ImGui::GetCursorPos();

    ImagePtr imageToUse = image;
    float2 uvMin = float2(0.0f, 0.0f);
    float2 uvMax = float2(1.0f, 1.0f);

    uint2 textureSize = uint2(64, 64);
    uint4 textureRegion = materialGetTextureRegion(data->material, type);
    float32 textureBlendingFactor = materialGetTextureBlendingFactor(data->material, type);
    bool8 textureEnabled = materialIsTextureEnabled(data->material, type);

    if(image == ImagePtr(nullptr))
    {
      uvMin = float2(defaultUVRect.x, defaultUVRect.y);
      uvMax = float2(defaultUVRect.z, defaultUVRect.w);

      imageToUse = defaultImage;
    }
    else
    {
      textureSize = imageGetSize(image);
      float4 imageRect = calculateUVRect(textureSize,
                                         uint2(textureRegion.x, textureRegion.y),
                                         uint2(textureRegion.z, textureRegion.w));

      uvMin = float2(imageRect.x, imageRect.y);
      uvMax = float2(imageRect.z, imageRect.w);      
    }

    ImGui::Image((void*)imageGetGLHandle(imageToUse), ImVec2(imageSize, imageSize), uvMin, uvMax);

    ImGui::SetCursorPos(imageStart);

    if(ImGui::InvisibleButton("Disable", float2(imageSize, imageSize)))
    {
      textureEnabled = textureEnabled == TRUE ? FALSE : TRUE;
      materialSetEnabledTexture(data->material, type, textureEnabled);
    }

    ImGui::SetCursorPos(imageStart);
    
    if(textureEnabled == FALSE)
    {
      ImGui::Image((void*)imageGetGLHandle(defaultImage), ImVec2(imageSize, imageSize),
                   float2(cancelUVRect.x, cancelUVRect.y), float2(cancelUVRect.z, cancelUVRect.w));
    }

    ImGui::BeginDisabled(textureEnabled == FALSE);
      
      ImGui::SameLine();

      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, float2(0.0f, 4.0f));
        ImGui::InputTextWithHint("", "path to a file", data->texturePaths[type], 256);

        ImGui::SameLine();

        if(textureEnabled == TRUE)
        {
          ImGui::BeginDisabled(strlen(data->texturePaths[type]) == 0);
        }

          if(ImGui::Button("Load"))
          {
            ImagePtr loadedImage = imageManagerLoadImage(data->texturePaths[type]);
            if(loadedImage != ImagePtr(nullptr))
            {
              uint2 loadedTextureSize = imageGetSize(loadedImage);
              uint4 loadedTextureRegion = uint4(0, 0, loadedTextureSize.x, loadedTextureSize.y);
              
              materialSetTexture(data->material, type, loadedImage);
              materialSetTextureRegion(data->material, type, loadedTextureRegion);
            }
          }

        if(textureEnabled == TRUE)
        {
          ImGui::EndDisabled();
        }
        

      ImGui::PopStyleVar();
      ImGui::NewLine();

      ImGui::SetCursorPosY(imageStart.y + imageSize * 0.4f);
      ImGui::Dummy(float2(imageSize, 0.0f));
      ImGui::SameLine();

      ImGui::PushStyleColor(ImGuiCol_FrameBg, float4(0.13f, 0.13f, 0.13f, 0.54f));

        uint32 minRegionSize = 0;      
        uint32 maxRegionSize = max(textureSize.x, textureSize.y);

        if(ImGui::SliderScalarN("Region", ImGuiDataType_U32, &textureRegion[0], 4, &minRegionSize, &maxRegionSize))
        {
          textureRegion.x = min(textureSize.x, textureRegion.x);
          textureRegion.y = min(textureSize.y, textureRegion.y);          
          textureRegion.z = min((int32)textureRegion.z, (int32)textureSize.x - (int32)textureRegion.x);
          textureRegion.w = min((int32)textureRegion.w, (int32)textureSize.y - (int32)textureRegion.y);          

          materialSetTextureRegion(data->material, type, textureRegion);
        }
      ImGui::PopStyleColor();

      ImGui::SetCursorPosY(imageStart.y + imageSize * 0.8f);
      ImGui::Dummy(float2(imageSize, 0.0f));
      ImGui::SameLine();

      if(ImGui::SliderFloat("Blending", &textureBlendingFactor, 1.0f, 10.0f))
      {
        materialSetTextureBlendingFactor(data->material, type, textureBlendingFactor);
      }
      
      ImGui::NewLine();

      ImGui::SetCursorPos(imageStart);
      ImGui::Dummy(float2(imageSize, imageSize));

      ImGui::Text("%s", materialTextureTypeLabel(type));
      ImGui::NewLine();

    ImGui::EndDisabled();

    ImGui::PopID();
  }

  // --- Colors settings ------------------------------------------------------
  
  ImGui::Text("Constant colors settings");
  ImGui::Separator();
  
  float4 ambientColor = materialGetAmbientColor(data->material);
  float4 diffuseColor = materialGetDiffuseColor(data->material);
  float4 specularColor = materialGetSpecularColor(data->material);
  float4 emissionColor = materialGetEmissionColor(data->material);

  // Ambient color
  pushIconSmallButtonStyle();
    if(ImGui::SmallButton(ICON_KI_RELOAD_INVERSE"##ReloadAmbient"))
    {
      materialSetAmbientColor(data->material, float4(0.0f, 0.0f, 0.0f, 1.0f));
    }

  ImGui::SameLine();
  popIconSmallButtonStyle();
  
  if(ImGui::ColorEdit3("Ambient color", &ambientColor[0]))
  {
    materialSetAmbientColor(data->material, ambientColor);
  }

  // Diffuse color  
  pushIconSmallButtonStyle();
    if(ImGui::SmallButton(ICON_KI_RELOAD_INVERSE"##ReloadDiffuse"))
    {
      materialSetDiffuseColor(data->material, float4(1.0f, 1.0f, 1.0f, 1.0f));
    }

  ImGui::SameLine();
  popIconSmallButtonStyle();
  
  if(ImGui::ColorEdit3("Diffuse color", &diffuseColor[0]))
  {
    materialSetDiffuseColor(data->material, diffuseColor);
  }

  // Specular color  
  pushIconSmallButtonStyle();
    if(ImGui::SmallButton(ICON_KI_RELOAD_INVERSE"##ReloadSpecular"))
    {
      materialSetSpecularColor(data->material, float4(1.0f, 1.0f, 1.0f, 1.0f));
    }

  ImGui::SameLine();
  popIconSmallButtonStyle();
  
  if(ImGui::ColorEdit3("Specular color", &specularColor[0]))
  {
    materialSetSpecularColor(data->material, specularColor);
  }

  // Emission color
  pushIconSmallButtonStyle();
    if(ImGui::SmallButton(ICON_KI_RELOAD_INVERSE"##ReloadEmission"))
    {
      materialSetEmissionColor(data->material, float4(0.0f, 0.0f, 0.0f, 1.0f));
    }

  ImGui::SameLine();
  popIconSmallButtonStyle();  

  if(ImGui::ColorEdit3("Emission color", &emissionColor[0]))
  {
    materialSetEmissionColor(data->material, emissionColor);
  }

  ImGui::NewLine();
  
  // --- Misc settings ------------------------------------------------------  

  ImGui::Text("Misc settings");
  ImGui::Separator();
  
  float32 metallic = materialGetMetallic(data->material);
  float32 roughness = materialGetRoughness(data->material);  
  float32 ior = materialGetIOR(data->material);
  float32 ao = materialGetAO(data->material);

  // Metallic
  pushIconSmallButtonStyle();
    if(ImGui::SmallButton(ICON_KI_RELOAD_INVERSE"##ReloadMetallic"))
    {
      materialSetMetallic(data->material, 0.0f);
    }

  ImGui::SameLine();
  popIconSmallButtonStyle();  

  if(ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f))
  {
    materialSetMetallic(data->material, metallic);
  }

  // Roughness  
  pushIconSmallButtonStyle();
    if(ImGui::SmallButton(ICON_KI_RELOAD_INVERSE"##ReloadRoughness"))
    {
      materialSetRoughness(data->material, 1.0f);
    }

  ImGui::SameLine();
  popIconSmallButtonStyle();  

  if(ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f))
  {
    materialSetRoughness(data->material, roughness);
  }  

  // Index of refraction
  pushIconSmallButtonStyle();
    if(ImGui::SmallButton(ICON_KI_RELOAD_INVERSE"##ReloadIOR"))
    {
      materialSetIOR(data->material, 1.0f);
    }

  ImGui::SameLine();
  popIconSmallButtonStyle();  

  if(ImGui::SliderFloat("IOR", &ior, 1.0f, 100.0f))
  {
    materialSetIOR(data->material, ior);
  }
  
  // Ambient occlusion
  pushIconSmallButtonStyle();
    if(ImGui::SmallButton(ICON_KI_RELOAD_INVERSE"##ReloadAO"))
    {
      materialSetAO(data->material, 0.0f);
    }

  ImGui::SameLine();
  popIconSmallButtonStyle();  

  if(ImGui::SliderFloat("AO", &ao, 0.0f, 1.0f))
  {
    materialSetAO(data->material, ao);
  }
  
  ImGui::PopStyleColor(5);
  ImGui::PopStyleVar();
}

void materialSettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{

}
