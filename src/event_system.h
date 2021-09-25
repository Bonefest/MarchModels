#pragma once

#include "defines.h"

enum EventType
{
  /** Content:
   * i32[0] = key
   * i32[1] = scancode
   * i32[2] = action
   * i32[3] = mods
   */
  EVENT_TYPE_KEY_PRESSED,

  /** Content:
   * i32[0] = key
   * i32[1] = scancode
   * i32[2] = action
   * i32[3] = mods
   */
  EVENT_TYPE_KEY_RELEASED,

  /** Content:
   * i32[0] = button
   * i32[1] = action
   * i32[2] = mods
   */  
  EVENT_TYPE_BUTTON_PRESSED,

  /** Content:
   * i32[0] = button
   * i32[1] = action
   * i32[2] = mods
   */    
  EVENT_TYPE_BUTTON_RELEASED,

  /** Content:
   * f32[0] = xpos
   * f32[1] = ypos
   */
  EVENT_TYPE_CURSOR_MOVED,

  /** Content:
   * i32[0] = width
   * i32[1] = height
   */
  EVENT_TYPE_WINDOW_RESIZED,
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

ENGINE_API bool8 registerListener(EventType eventType, void* listener, fpListenerCallback callback);
ENGINE_API bool8 unregisterListener(EventType eventType, void* listener);

/** Notifies all registered listeners immediately. It's not a recommended way of communication */
ENGINE_API void triggerEvent(EventType eventType, EventData eventData, void* sender = nullptr);

/** 
 * Pushes events on the queue. Events then can be processed through pollEvent().
 * 
 * @note Current implementation executes pollEvent() automatically.
 */
ENGINE_API void pushEvent(EventType eventType, EventData eventData, void* sender = nullptr);
ENGINE_API bool8 pollEvent(EventData* outEventData, void** outSender = nullptr);



