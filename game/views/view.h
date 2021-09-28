#pragma once

#include <string>

#include <defines.h>
#include <maths/common.h>
#include <event_system.h>

struct View
{
  bool8 (*initialize)();
  void (*shutdown)();

  void (*onLoad)();
  void (*onUnload)();
  void (*onResize)(uint2 newSize);

  void (*update)(float64 delta);  
  void (*draw)(float64 delta);
  void (*processInput)(const EventData& eventData, void* sender);

  std::string name;
  void* internalData;
};

void destroyView(View* view);
