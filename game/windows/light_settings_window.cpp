#include "utils.h"
#include "ui_utils.h"
#include "ui_styles.h"
#include "editor_utils.h"
#include "light_settings_window.h"

using std::string;

struct LightSettingsWindowData
{
  AssetPtr lightSource;
};

static bool8 lightSettingsWindowInitialize(Window*);
static void lightSettingsWindowShutdown(Window*);
static void lightSettingsWindowUpdate(Window* window, float64 delta);
static void lightSettingsWindowDraw(Window* window, float64 delta);
static void lightSettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender);

bool8 createLightSettingsWindow(AssetPtr lightSource, Window** outWindow)
{
  WindowInterface interface = {};
  interface.initialize = lightSettingsWindowInitialize;
  interface.shutdown = lightSettingsWindowShutdown;
  interface.update = lightSettingsWindowUpdate;
  interface.draw = lightSettingsWindowDraw;
  interface.processInput = lightSettingsWindowProcessInput;

  if(allocateWindow(interface, lightSettingsWindowIdentifier(lightSource), outWindow) == FALSE)
  {
    return FALSE;
  }

  LightSettingsWindowData* data = engineAllocObject<LightSettingsWindowData>(MEMORY_TYPE_GENERAL);
  data->lightSource = lightSource;
  windowSetInternalData(*outWindow, data);
  
  return TRUE;
}

string lightSettingsWindowIdentifier(Asset* lsource)
{
  char identifier[255];
  sprintf(identifier, "%s settings##%p", assetGetName(lsource).c_str(), lsource);

  return identifier;
}

bool8 lightSettingsWindowInitialize(Window* window)
{
  return TRUE;
}

void lightSettingsWindowShutdown(Window* window)
{
  LightSettingsWindowData* data = (LightSettingsWindowData*)windowGetInternalData(window);
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

void lightSettingsWindowUpdate(Window* window, float64 delta)
{

}

void lightSettingsWindowDraw(Window* window, float64 delta)
{
  LightSettingsWindowData* data = (LightSettingsWindowData*)windowGetInternalData(window);

  float32 avalWidth = ImGui::GetContentRegionAvail().x * 0.7;
  
  LightSourceParameters& parameters = lightSourceGetParameters(data->lightSource);

  static const char* lightSourceTypesLabels[] =
  {
    "Directional light",
    "Spotlight",
    "Point light"
  };

  ImGui::PushItemWidth(avalWidth);
  ImGui::Combo("Light type", (int32*)&parameters.type, lightSourceTypesLabels, ARRAY_SIZE(lightSourceTypesLabels));

  ImGui::PushItemWidth(avalWidth * 0.1);
  ImGui::Checkbox("Shadow", (bool*)&parameters.shadowEnabled);
  ImGui::PopItemWidth();
  ImGui::SameLine();
  
  ImGui::BeginDisabled(parameters.shadowEnabled != TRUE);
  ImGui::PushItemWidth(avalWidth * 0.8);
    ImGui::SliderFloat("Shadow sharpness factor", &parameters.shadowFactor, 0.0f, 200.0f);
  ImGui::PopItemWidth();
  ImGui::EndDisabled();
  
  ImGui::SliderFloat3("Position", &parameters.position[0], -10.0f, 10.0f);
  if(ImGui::SliderFloat3("Direction", &parameters.forward[0], -1.0f, 1.0f))
  {
    parameters.forward = float4(linalg::normalize(parameters.forward.xyz()), 0.0);
  }

  ImGui::PushItemWidth(avalWidth * 0.5);
    ImGui::SliderFloat("Linear distance attenuation factor",
                       &parameters.attenuationDistanceFactors[0], 0.01f, 10.0f);
    ImGui::SliderFloat("Quadratic distance attenuation factor",
                       &parameters.attenuationDistanceFactors[1], 0.01f, 10.0f);

    parameters.attenuationAngleFactors[0] = acos(parameters.attenuationAngleFactors[0]);
    parameters.attenuationAngleFactors[1] = acos(parameters.attenuationAngleFactors[1]);
    
    ImGui::SliderAngle("Cutoff start angle", &parameters.attenuationAngleFactors[0], 0.0f, 90.0f);
    ImGui::SliderAngle("Cutoff end angle", &parameters.attenuationAngleFactors[1], 0.0f, 90.0f);

    parameters.attenuationAngleFactors[1] = std::max(parameters.attenuationAngleFactors[0],
                                                     parameters.attenuationAngleFactors[1]);

    parameters.attenuationAngleFactors[0] = cos(parameters.attenuationAngleFactors[0]);
    parameters.attenuationAngleFactors[1] = cos(parameters.attenuationAngleFactors[1]);
  ImGui::PopItemWidth();

  ImGui::ColorEdit4("Intensity", &parameters.intensity[0]);
  
  ImGui::PopItemWidth();
}

void lightSettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{

}
