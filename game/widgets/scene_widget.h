#include "widget.h"

enum RenderingMode
{
  FAST_DEBUG,
  SIMPLE,
  BEAUTY
};

bool8 createSceneWidget(Widget** widget);

void sceneWidgetSetRenderingMode(Widget* widget, RenderingMode renderingMode);
RenderingMode sceneWidgetGetRenderingMode(Widget* widget);
void sceneWidgetSetFastDebugParameters(Widget* widget, uint32 xSpace, uint32 ySpace);
uint2 sceneWidgetGetFastDebugParameters(Widget* widget);

void sceneWidgetSetEnabledRealtimeRendering(Widget* widget, bool8 enabled);
bool8 sceneWidgetIsRealtimeRenderingEnabled(Widget* widget);

void sceneWidgetSetCamera(Widget* widget, Camera* camera);
// void sceneWidgetSetScene(Widget* widget, Scene* scene);
void sceneWidgetSetShape(Widget* widget, Shape* shape);

// Integration of the scene is not the part of the scene_widget. We should have something like
// DebugIntegrator, FastIntegrator etc.
