#include <memory_manager.h>

#include "image_integrator_display_widget.h"

struct ImageIntegratorDisplayWidgetData
{
  std::string identifier;
  ImageIntegrator* integrator;
  float32 maxFPS;
  float32 timePerFrame;
  float32 elapsedTime;
};

static bool8 initializeImageIntegratorDisplayWidget(Widget* widget)
{
  return TRUE;
}

static void shutdownImageIntegratorDisplayWidget(Widget* widget)
{
  ImageIntegratorDisplayWidgetData* data = (ImageIntegratorDisplayWidgetData*)widgetGetInternalData(widget);
  engineFreeMem(data, MEMORY_TYPE_GENERAL);
}

static void updateImageIntegratorDisplayWidget(Widget* widget, View* view, float64 delta)
{
  ImageIntegratorDisplayWidgetData* data = (ImageIntegratorDisplayWidgetData*)widgetGetInternalData(widget);
  data->elapsedTime += delta;
}

static void drawImageIntegratorDisplayWidget(Widget* widget, View* view, float64 delta)
{
  ImageIntegratorDisplayWidgetData* data = (ImageIntegratorDisplayWidgetData*)widgetGetInternalData(widget);  
  if(data->elapsedTime > data->timePerFrame)
  {
    imageIntegratorExecute(data->integrator, glfwGetTime());
    data->elapsedTime = 0.0f;
  }

  Film* film = imageIntegratorGetFilm(data->integrator);
  uint2 filmSize = filmGetSize(film);
  
  ImGui::Begin(data->identifier.c_str());
  ImGui::Image((void*)filmGetGLTexture(film),
               ImVec2(filmSize.x, filmSize.y), ImVec2(1.0f, 1.0f), ImVec2(0.0f, 0.0f));  
  ImGui::End();
}

static void processInputImageIntegratorDisplayWidget(Widget* widget,
                                                     View* view,
                                                     const EventData& eventData,
                                                     void* sender)
{
  
}

bool8 createImageIntegratorDisplayWidget(const std::string& identifier,
                                         ImageIntegrator* integrator,
                                         float32 maxFPS,
                                         Widget** outWidget)
{
  WidgetInterface interface = {};
  interface.initialize = initializeImageIntegratorDisplayWidget;
  interface.shutdown = shutdownImageIntegratorDisplayWidget;
  interface.update = updateImageIntegratorDisplayWidget;
  interface.draw = drawImageIntegratorDisplayWidget;
  interface.processInput = processInputImageIntegratorDisplayWidget;

  if(allocateWidget(interface, outWidget) == FALSE)
  {
    return FALSE;
  }

  ImageIntegratorDisplayWidgetData* data = engineAllocObject<ImageIntegratorDisplayWidgetData>(MEMORY_TYPE_GENERAL);
  data->identifier = identifier;
  data->integrator = integrator;
  data->elapsedTime = 0.0f;

  widgetSetInternalData(*outWidget, data);
  imageIntegratorDisplayWidgetSetMaxFPS(*outWidget, maxFPS);

  return TRUE;
}


void imageIntegratorDisplayWidgetSetMaxFPS(Widget* widget, float32 maxFPS)
{
  ImageIntegratorDisplayWidgetData* data = (ImageIntegratorDisplayWidgetData*)widgetGetInternalData(widget);
  data->maxFPS = maxFPS;
  data->timePerFrame = 1.0f / maxFPS;
}

float32 imageIntegratorDisplayWidgetGetMaxFPS(Widget* widget)
{
  ImageIntegratorDisplayWidgetData* data = (ImageIntegratorDisplayWidgetData*)widgetGetInternalData(widget);
  return data->maxFPS;
}
