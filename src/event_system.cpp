#include <queue>
#include <vector>

#include "logging.h"

#include "event_system.h"

using std::queue;
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
static queue<PollEventData> polledEvents;

bool8 initEventSystem()
{

  return TRUE;
}

void shutdownEventSystem()
{

}

bool8 registerListener(EventType eventType, void* plistener, fpListenerCallback callback)
{
  assert((uint32)eventType < EVENT_TYPE_MAX);

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

void triggerEvent(EventType eventType, EventData eventData, void* sender)
{
  assert((uint32)eventType < EVENT_TYPE_MAX);

  eventData.type = eventType;
  
  for(Listener& listener: listeners[eventType])
  {
    if(listener.callback(eventData, sender, listener.plistener) == TRUE)
    {
      return;
    }
  }
}

void pushEvent(EventType eventType, EventData eventData, void* sender)
{
  assert((uint32)eventType < EVENT_TYPE_MAX);
  
  eventData.type = eventType;
  polledEvents.push(PollEventData{eventData, sender});
}

bool8 pollEvent(EventData* outEventData, void** outSender)
{
  if(polledEvents.empty())
  {
    return FALSE;
  }

  PollEventData pollEventData = polledEvents.front();
  *outEventData = pollEventData.data;
  polledEvents.pop();

  if(outSender != nullptr)
  {
    *outSender = pollEventData.sender;
  }

  return TRUE;
}


