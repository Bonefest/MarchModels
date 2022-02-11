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
    ImGui::SliderFloat("Shadow factor", &parameters.shadowFactor, 0.0f, 200.0f);
  ImGui::PopItemWidth();
  ImGui::EndDisabled();
  
  ImGui::SliderFloat3("Position", &parameters.position[0], -10.0f, 10.0f);
  if(ImGui::SliderFloat4("Orientation", &parameters.orientation[0], -1.0f, 1.0f))
  {
    parameters.orientation = linalg::normalize(parameters.orientation);
  }

  ImGui::PushItemWidth(avalWidth * 0.5);
    ImGui::SliderFloat("Linear distance attenuation", &parameters.attenuationDistance[0], 0.01f, 10.0f);
    ImGui::SliderFloat("Quadratic distance attenuation", &parameters.attenuationDistance[1], 0.01f, 10.0f);

    ImGui::SliderAngle("Cutoff start angle", &parameters.attenuationAngle[0], 0.0f, 90.0f);
    ImGui::SliderAngle("Cutoff end angle", &parameters.attenuationAngle[1], 0.0f, 90.0f);

    parameters.attenuationAngle[1] = std::max(parameters.attenuationAngle[0],
                                              parameters.attenuationAngle[1]);
  ImGui::PopItemWidth();

  ImGui::ColorEdit4("Intensity", &parameters.intensity[0]);
  
  ImGui::PopItemWidth();
}

void lightSettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{

}
