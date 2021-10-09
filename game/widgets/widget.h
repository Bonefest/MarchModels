#pragma once

#include "views/view.h"

struct Widget;

struct WidgetInterface
{
  bool8 (*initialize)(Widget*);
  void (*shutdown)(Widget*);
  void (*update)(Widget* widget, View* view, float64 delta);
  void (*draw)(Widget* widget, View* view, float64 delta);
  void (*processInput)(Widget* widget, View* view, const EventData& eventData, void* sender);
};

bool8 allocateWidget(WidgetInterface interface, const std::string& identifier, Widget** outWidget);
void freeWidget(Widget* widget);

bool8 initializeWidget(Widget* widget);
void updateWidget(Widget* widget, View* view, float64 delta);
void drawWidget(Widget* widget, View* view, float64 delta);
void processInputWidget(Widget* widget, View* view, const EventData& eventData, void* sender);

const std::string& widgetGetIdentifier(Widget* widget);

void widgetSetInternalData(Widget* widget, void* internalData);
void* widgetGetInternalData(Widget* widget);
