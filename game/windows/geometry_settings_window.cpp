#include "utils.h"
#include "ui_utils.h"
#include "ui_styles.h"
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

  const char* geometryTypeLabel = geometryIsRoot(data->geometry)   ? "root" :
                                  geometryIsBranch(data->geometry) ? "branch" :
                                                                     "leaf";

  pushIconSmallButtonStyle();
    if(ImGui::SmallButton("[?]"))
    {
      ImGui::BeginTooltip();
        ImGui::TextColored("_<C>%#010x</C>_[%s] _<C>%#010x</C>_'%s'_<C>0x1</C>_ was created on_<C>%#010x</C>_ 12.12.2021",
                           revbytes((ImU32)HighlightPrimaryClr),
                           geometryTypeLabel,
                           revbytes((ImU32)HighlightSecondaryClr),
                           assetGetName(data->geometry).c_str(),
                           revbytes((uint32)HighlightSecondaryClr));

      ImGui::EndTooltip();
    }

    ImGui::SameLine();
    
    ImGui::SmallButton("[Export]");
    ImGui::SameLine();
    ImGui::SmallButton("[Import]");
    ImGui::SameLine();
    ImGui::SmallButton("[Load]");
    ImGui::SameLine();
    ImGui::SmallButton("[Save as]");
    ImGui::SameLine();
    ImGui::SmallButton("[Delete]");
  popIconSmallButtonStyle();
  
  // [type] 'name' was created on 'date'

  ImGui::Spacing();
  
  // Position input
  float3 geometryPosition = geometryGetPosition(data->geometry);
  
  const char* relModePositionIcon = data->positionRelativeToParent == TRUE ?
    ICON_KI_USER"##PositionRelMode" : ICON_KI_USERS"##PositionRelMode";

  pushIconButtonStyle();
    if(ImGui::Button(relModePositionIcon))
    {
      data->positionRelativeToParent = !data->positionRelativeToParent;
    }

    ImGui::SameLine();

    if(ImGui::Button(ICON_KI_RELOAD_INVERSE"##ReloadPosition"))
    {
      geometryPosition = float3();
    }
  popIconButtonStyle();
    
  ImGui::SameLine();
  

  ImGui::SliderFloat3("Position##Geometry", &geometryPosition.x, -10.0, 10.0);
  geometrySetPosition(data->geometry, geometryPosition);

  // Orientation input
  quat geometryOrientation = geometryGetOrientation(data->geometry);
  
  const char* relModeOrientationIcon = data->orientationRelativeToParent == TRUE ?
    ICON_KI_USER"##OrientationRelMode" : ICON_KI_USERS"##OrientationRelMode";

  pushIconButtonStyle();  
    if(ImGui::Button(relModeOrientationIcon))
    {
      data->orientationRelativeToParent = !data->orientationRelativeToParent;
    }

    ImGui::SameLine();


    if(ImGui::Button(ICON_KI_RELOAD_INVERSE"##ReloadOrientation"))
    {
      geometryOrientation = rotation_quat(float3(0.0f, 0.0f, 1.0f), (float32)ONE_PI);
    }
  popIconButtonStyle();

  ImGui::SameLine();
  

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

  // combination function
  // sdf, idfs, odfs
  // meta information (creation date)
  // list of children
}

void geometrySettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{

}
