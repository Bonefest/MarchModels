#pragma once

#include "defines.h"

enum EventType
{
  EVENT_TYPE_KEY_PRESSED,
  EVENT_TYPE_KEY_RELEASED,
  EVENT_TYPE_BUTTON_PRESSED,
  EVENT_TYPE_BUTTON_RELEASED,
  EVENT_TYPE_CURSOR_MOVED,
  EVENT_TYPE_COUNT,

  EVENT_TYPE_USER_DEFINED,
  EVENT_TYPE_MAX = 127
};

struct EventData
{
  EventType type;
  
  union
  {
    struct
    {
      bool8   b[8];
      int32   i32[8];    
      uint32  u32[8];
      float32 f32[8];
      char    c[32];
    };
    
    char bytes[64];
  };
};

typedef bool8(*fpListenerCallback)(EventData eventData, void* sender, void* listener);

bool8 initEventSystem();
void shutdownEventSystem();

EDITOR_API bool8 registerListener(EventType eventType, void* listener, fpListenerCallback callback);
EDITOR_API bool8 unregisterListener(EventType eventType, void* listener);

EDITOR_API void triggerEvent(EventType eventType, EventData eventData, void* sender = nullptr);
EDITOR_API void pushEvent(EventType eventType, EventData eventData, void* sender = nullptr);
EDITOR_API bool8 pollEvent(EventData* outEventData, void** outSender = nullptr);



