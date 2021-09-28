#pragma once

struct Widget
{
  void (*initialize);
  void (*shutdown);
  void (*update)(View* view, float64 delta);
  void (*draw)(View* view, float64 delta);
  void (*processInput)(View* view, const EventData& eventData, void* sender);

  void* internalData;
};

bool8 allocateWidget(Widget** outWidget);
void freeWidget(Widget* widget);


