#pragma once

#include <gtest/gtest.h>
#include <event_system.h>

static uint32 listener = 0;
bool8 somePassthroughListener(EventData eventData, void* sender, void* listener) { ::listener++; return FALSE; }
bool8 someStopListener(EventData eventData, void* sender, void* listener) { ::listener++; return TRUE; }

class EventSystemTests: public ::testing::Test
{
protected:
  void SetUp() override
  {
    shutdownEventSystem();
    initEventSystem();
    listener = 0;
  }
};

TEST_F(EventSystemTests, ListenSameTypeTwice)
{
  EXPECT_EQ(registerListener(EVENT_TYPE_KEY_PRESSED, &listener, somePassthroughListener), TRUE);
  EXPECT_EQ(registerListener(EVENT_TYPE_KEY_PRESSED, &listener, somePassthroughListener), FALSE);
}

TEST_F(EventSystemTests, UnregisterExistingListener)
{
  EXPECT_EQ(registerListener(EVENT_TYPE_KEY_PRESSED, &listener, somePassthroughListener), TRUE);  
  EXPECT_EQ(unregisterListener(EVENT_TYPE_KEY_PRESSED, &listener), TRUE);  
}

TEST_F(EventSystemTests, UnregisterNotExistingListener)
{
  EXPECT_EQ(unregisterListener(EVENT_TYPE_KEY_PRESSED, &listener), FALSE);  
}

TEST_F(EventSystemTests, ListenSameTypeTwiceWithReregister)
{
  EXPECT_EQ(registerListener(EVENT_TYPE_KEY_PRESSED, &listener, somePassthroughListener), TRUE);
  EXPECT_EQ(unregisterListener(EVENT_TYPE_KEY_PRESSED, &listener), TRUE);
  EXPECT_EQ(registerListener(EVENT_TYPE_KEY_PRESSED, &listener, somePassthroughListener), TRUE);  
}

const static uint32 correctEventType = EVENT_TYPE_COUNT + 1;

TEST_F(EventSystemTests, RegisterWithCustomTypeWithinCorrectRange)
{
  EXPECT_EQ(registerListener((EventType)correctEventType, &listener, somePassthroughListener), TRUE);  
}

const static uint32 wrongEventType = EVENT_TYPE_MAX + 1;

TEST_F(EventSystemTests, RegisterWithCustomTypeOutOfRange)
{
  EXPECT_EQ(registerListener((EventType)wrongEventType, &listener, somePassthroughListener), FALSE);  
}

TEST_F(EventSystemTests, PassThroughListenerPassesThrough)
{
  uint32 secondListener = 0;
  EXPECT_EQ(registerListener(EVENT_TYPE_KEY_PRESSED, &listener, somePassthroughListener), TRUE);
  EXPECT_EQ(registerListener(EVENT_TYPE_KEY_PRESSED, &secondListener, someStopListener), TRUE);

  triggerEvent(EVENT_TYPE_KEY_PRESSED);

  EXPECT_EQ(listener, 2);
}

TEST_F(EventSystemTests, StopListenerNotPassesThrough)
{
  uint32 secondListener = 0;  
  EXPECT_EQ(registerListener(EVENT_TYPE_KEY_PRESSED, &listener, someStopListener), TRUE);  
  EXPECT_EQ(registerListener(EVENT_TYPE_KEY_PRESSED, &secondListener, somePassthroughListener), TRUE);

  triggerEvent(EVENT_TYPE_KEY_PRESSED);

  EXPECT_EQ(listener, 1);
}

TEST_F(EventSystemTests, PushNotExecutesImmediately)
{
  EXPECT_EQ(registerListener(EVENT_TYPE_KEY_PRESSED, &listener, someStopListener), TRUE);
  pushEvent(EVENT_TYPE_KEY_PRESSED);

  EXPECT_EQ(listener, 0);
}
            // register custom type
