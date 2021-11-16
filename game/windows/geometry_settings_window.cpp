#include "ui_utils.h"
#include "geometry_settings_window.h"

using std::string;

struct GeometrySettingsWindowData
{
  AssetPtr geometry;
  bool8 positionRelativeToParent;
  bool8 orientationRelativeToParent;
};

static bool8 geometrySettingsWindowInitialize(Window*);
static void geometrySettingsWindowShutdown(Window*);
static void geometrySettingsWindowUpdate(Window* window, float64 delta);
static void geometrySettingsWindowDraw(Window* window, float64 delta);
static void geometrySettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender);

bool8 createGeometrySettingsWindow(AssetPtr geometry, Window** outWindow)
{
  WindowInterface interface = {};
  interface.initialize = geometrySettingsWindowInitialize;
  interface.shutdown = geometrySettingsWindowShutdown;
  interface.update = geometrySettingsWindowUpdate;
  interface.draw = geometrySettingsWindowDraw;
  interface.processInput = geometrySettingsWindowProcessInput;

  if(allocateWindow(interface, geometrySettingsWindowIdentifier(geometry), outWindow) == FALSE)
  {
    return FALSE;
  }

  GeometrySettingsWindowData* data = engineAllocObject<GeometrySettingsWindowData>(MEMORY_TYPE_GENERAL);
  data->geometry = geometry;

  windowSetInternalData(*outWindow, data);
  
  return TRUE;
}

string geometrySettingsWindowIdentifier(Asset* geometry)
{
  char identifier[255];
  sprintf(identifier, "%s settings##%p", assetGetName(geometry).c_str(), geometry);

  return identifier;
}

bool8 geometrySettingsWindowInitialize(Window* window)
{
  ImGuiWindowFlags flags = windowGetFlags(window);
  flags |= ImGuiWindowFlags_MenuBar;
  windowSetFlags(window, flags);
  
  return TRUE;
}

void geometrySettingsWindowShutdown(Window* window)
{
  GeometrySettingsWindowData* data = (GeometrySettingsWindowData*)windowGetInternalData(window);
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

void geometrySettingsWindowUpdate(Window* window, float64 delta)
{

}

void geometrySettingsWindowDraw(Window* window, float64 delta)
{
  GeometrySettingsWindowData* data = (GeometrySettingsWindowData*)windowGetInternalData(window);

  if(ImGui::BeginMenuBar())
  {
    if(ImGui::BeginMenu("Reset"))
    {
      if(ImGui::MenuItem("All"))
      {

      }
      else if(ImGui::MenuItem("Position"))
      {

      }
      else if(ImGui::MenuItem("Orientation"))
      {

      }      
      ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
  }
  
  ImGui::Button("[Reset]");
  
  // Position input
  const char* relModePositionIcon = data->positionRelativeToParent == TRUE ?
    ICON_KI_USER"##PositionRelMode" : ICON_KI_USERS"##PositionRelMode";
  
  if(ImGui::Button(relModePositionIcon))
  {
    data->positionRelativeToParent = !data->positionRelativeToParent;
  }

  ImGui::SameLine();
  
  float3 geometryPosition = geometryGetPosition(data->geometry);
  ImGui::SliderFloat3("Position##Geometry", &geometryPosition.x, -10.0, 10.0);
  geometrySetPosition(data->geometry, geometryPosition);

  // Orientation input
  const char* relModeOrientationIcon = data->orientationRelativeToParent == TRUE ?
    ICON_KI_USER"##OrientationRelMode" : ICON_KI_USERS"##OrientationRelMode";

  if(ImGui::Button(relModeOrientationIcon))
  {
    data->orientationRelativeToParent = !data->orientationRelativeToParent;
  }

  ImGui::SameLine();
  
  quat geometryOrientation = geometryGetOrientation(data->geometry);
  float3 axis = qaxis(geometryOrientation);
  float32 angle = qangle(geometryOrientation);

  float4 axisAngle(axis, toDeg(angle));
  const static float32 axisAngleMinRange[] = {-1.0, -1.0, -1.0, 1.0};
  const static float32 axisAngleMaxRange[] = { 1.0,  1.0,  1.0, 359.0};
  
  ImGui::SliderScalarN("Axis, angle",
                       ImGuiDataType_Float,
                       &axisAngle,
                       4,
                       axisAngleMinRange,
                       axisAngleMaxRange,
                       "%.2f",
                       ImGuiSliderFlags_MultiRange);

  geometrySetOrientation(data->geometry, rotation_quat(normalize(axisAngle.xyz()), toRad(axisAngle.w)));
}

void geometrySettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{

}
