#include <logging.h>
#include <memory_manager.h>

#include "widget.h"

struct Widget
{
  WidgetInterface interface;
  std::string identifier;
  
  bool8 initialized;
  void* internalData;
};

bool8 allocateWidget(WidgetInterface interface, const std::string& identifier, Widget** outWidget)
{
  *outWidget = engineAllocObject<Widget>(MEMORY_TYPE_GENERAL);
  (*outWidget)->interface = interface;
  (*outWidget)->identifier = identifier;  
  (*outWidget)->initialized = FALSE;
  (*outWidget)->internalData = nullptr;
  
  return TRUE;
}

void freeWidget(Widget* widget)
{
  if(widget->initialized == TRUE)
  {
    widget->interface.shutdown(widget);
  }

  engineFreeObject(widget, MEMORY_TYPE_GENERAL);
}

bool8 initializeWidget(Widget* widget)
{
  if(widget->initialized == TRUE)
  {
    LOG_ERROR("Attempt to initiailize widget twice!");
    return FALSE;
  }

  widget->initialized = widget->interface.initialize(widget);
  return widget->initialized;
}

void updateWidget(Widget* widget, View* view, float64 delta)
{
  widget->interface.update(widget, view, delta);
}

void drawWidget(Widget* widget, View* view, float64 delta)
{
  widget->interface.draw(widget, view, delta);
}

void processInputWidget(Widget* widget, View* view, const EventData& eventData, void* sender)
{
  widget->interface.processInput(widget, view, eventData, sender);
}

const std::string& widgetGetIdentifier(Widget* widget)
{
  return widget->identifier;
}

void widgetSetInternalData(Widget* widget, void* internalData)
{
  widget->internalData = internalData;
}

void* widgetGetInternalData(Widget* widget)
{
  return widget->internalData;
}
