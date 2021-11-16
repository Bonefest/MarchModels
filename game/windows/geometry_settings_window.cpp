#include "geometry_settings_window.h"

using std::string;

struct GeometrySettingsWindowData
{
  AssetPtr geometry;
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

bool8 geometrySettingsWindowInitialize(Window* windo)
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
  
  float3 geometryPosition = geometryGetPosition(data->geometry);
  ImGui::SliderFloat3("Position##Geometry", &geometryPosition.x, -10.0, 10.0);
  geometrySetPosition(data->geometry, geometryPosition);

  quat geometryOrientation = geometryGetOrientation(data->geometry);
  float3 axis = qaxis(geometryOrientation);
  float32 angle = qangle(geometryOrientation);
  
  ImGui::SliderFloat3("Axis##GeometryOrientation", &axis.x, -1.0, 1.0);
  ImGui::SliderAngle("Angle##GeometryOrientation", &angle, 1.0, 359.0);

  geometrySetOrientation(data->geometry, rotation_quat(normalize(axis), angle));
}

void geometrySettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{

}