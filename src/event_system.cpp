#include <deque>
#include <vector>

#include "logging.h"

#include "event_system.h"

using std::deque;
using std::vector;

struct Listener
{
  void* plistener;
  fpListenerCallback callback;
};

struct PollEventData
{
  EventData data;
  void* sender;
};

static vector<Listener> listeners[EVENT_TYPE_MAX];
static deque<PollEventData> polledEvents;

bool8 initEventSystem()
{

  return TRUE;
}

void shutdownEventSystem()
{
  for(uint32 i = 0; i < EVENT_TYPE_MAX; i++)
  {
    listeners[i].clear();
  }

  polledEvents.clear();
}

bool8 registerListener(EventType eventType, void* plistener, fpListenerCallback callback)
{
  if((uint32)eventType >= EVENT_TYPE_MAX)
  {
    LOG_ERROR("Attempt to register a custom type with an ID higher than maximal allowed (%u >= %u)",
              (uint32)eventType, EVENT_TYPE_MAX);
    return FALSE;
  }

  for(Listener& listener: listeners[eventType])
  {
    if(listener.plistener == plistener)
    {
      LOG_WARNING("Attempt to register same listener for the same event type twice!");
      return FALSE;
    }
  }

  listeners[eventType].push_back(Listener{plistener, callback});
  
  return TRUE;
}

bool8 unregisterListener(EventType eventType, void* listener)
{
  assert((uint32)eventType < EVENT_TYPE_MAX);

  for(auto listenerIt = listeners[eventType].begin();
      listenerIt != listeners[eventType].end();
      listenerIt++)
  {
    if(listenerIt->plistener == listener)
    {
      listeners[eventType].erase(listenerIt);
      return TRUE;
    }
  }

  LOG_WARNING("Attempt to unregister a not registered listener!");
  return FALSE;
}

void triggerEvent(EventData eventData, void* sender)
{
  assert((uint32)eventData.type < EVENT_TYPE_MAX);

  for(Listener& listener: listeners[eventData.type])
  {
    if(listener.callback(eventData, sender, listener.plistener) == TRUE)
    {
      return;
    }
  }
}

void triggerEvent(EventType eventType, void* sender)
{
  triggerEvent(EventData{eventType}, sender);
}

void pushEvent(EventData eventData, void* sender)
{
  assert((uint32)eventData.type < EVENT_TYPE_MAX);
  
  polledEvents.push_back(PollEventData{eventData, sender});
}

void pushEvent(EventType eventType, void* sender)
{
  pushEvent(EventData{eventType}, sender);
}

bool8 pollEvent(EventData* outEventData, void** outSender)
{
  if(polledEvents.empty())
  {
    return FALSE;
  }

  PollEventData pollEventData = polledEvents.front();
  *outEventData = pollEventData.data;
  polledEvents.pop_front();

  if(outSender != nullptr)
  {
    *outSender = pollEventData.sender;
  }

  return TRUE;
}


