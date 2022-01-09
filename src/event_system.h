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
  
  /** Content:
   * u32[0] = log type
   * u32[1] = message length
   * ptr[0] = char* ptr to message
   *
   * @warning: message should be copied!
   */
  EVENT_TYPE_LOG_MESSAGE,

  /**
   * u32[0] = message length
   * ptr[0] = char* ptr to message
   *
   * @warning: message should be copied!
   */
  EVENT_TYPE_CONSOLE_MESSAGE,

  /**
   * ptr[0] = window
   */
  EVENT_TYPE_WINDOW_CREATED,

  /**
   * ptr[0] = window
   *
   * @warning: given window is already destroyed, it should be used
   * only in case if it was stored somewhere else
   */
  EVENT_TYPE_WINDOW_DESTROYED,
  
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

      void*   ptr[4];      
    };
    
    char bytes[64 + sizeof(void*) * 4];
  };
};

/**
 * @return TRUE if message was processed and it should not be passed to other listeners,
 * FALSE if message was/wasn't processed but it can be passed to other listeners too
 */
typedef bool8(*fpListenerCallback)(EventData eventData, void* sender, void* listener);

bool8 initEventSystem();
void shutdownEventSystem();

ENGINE_API bool8 registerListener(EventType eventType, void* listener, fpListenerCallback callback);
ENGINE_API bool8 unregisterListener(EventType eventType, void* listener);

/** Notifies all registered listeners immediately. It's not a recommended way of communication */
ENGINE_API void triggerEvent(EventData eventData, void* sender = nullptr);
ENGINE_API void triggerEvent(EventType eventType, void* sender = nullptr);

/** 
 * Pushes events on the queue. Events then can be processed through pollEvent().
 * 
 * @note Current implementation executes pollEvent() automatically.
 */
ENGINE_API void pushEvent(EventData eventData, void* sender = nullptr);

/**
 * Same as previous pushEvent(...) function but for events that do not have attached data
 */
ENGINE_API void pushEvent(EventType eventType, void* sender = nullptr);
ENGINE_API bool8 pollEvent(EventData* outEventData, void** outSender = nullptr);



