#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <windows/window_manager.h>

struct MockedWindow
{
  uint32 initializeRefCount = 0;
  uint32 shutdownRefCount = 0;
  uint32 updateRefCount = 0;
  uint32 drawRefCount = 0;
  uint32 processInputRefCount = 0;
};

bool8 mockedWindowInitialize(Window* window)
{
  MockedWindow* data = (MockedWindow*)windowGetInternalData(window);
  data->initializeRefCount++;
  return TRUE;
}

void mockedWindowShutdown(Window* window)
{
  MockedWindow* data = (MockedWindow*)windowGetInternalData(window);
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

void mockedWindowUpdate(Window* window, float64 delta)
{
  MockedWindow* data = (MockedWindow*)windowGetInternalData(window);
  data->updateRefCount++;
}

void mockedWindowDraw(Window* window, float64 delta)
{
  MockedWindow* data = (MockedWindow*)windowGetInternalData(window);
  data->drawRefCount++;
}

void mockedWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{
  MockedWindow* data = (MockedWindow*)windowGetInternalData(window);
  data->processInputRefCount++;
}

Window* createMockedWindow()
{
  WindowInterface interface = {};
  interface.initialize = mockedWindowInitialize;
  interface.shutdown = mockedWindowShutdown;
  interface.update = mockedWindowUpdate;
  interface.draw = mockedWindowDraw;
  interface.processInput = mockedWindowProcessInput;
  interface.usesCustomDrawPipeline = TRUE;

  Window* window = nullptr;
  allocateWindow(interface, "mocked_window", &window);

  MockedWindow* data = engineAllocObject<MockedWindow>(MEMORY_TYPE_GENERAL);
  windowSetInternalData(window, data);

  return window;
}

class WindowManagerTests: public ::testing::Test
{
protected:
  void SetUp() override
  {
    initWindowManager();

    mockedWindow = WindowPtr(createMockedWindow());

  }

  void TearDown() override
  {
    shutdownWindowManager();
  }

  WindowPtr mockedWindow;
};

TEST_F(WindowManagerTests, DefaultInit)
{
  windowManagerAddWindow(mockedWindow);  
  MockedWindow* data = (MockedWindow*)windowGetInternalData(mockedWindow);
  EXPECT_EQ(data->initializeRefCount, 1);
}

TEST_F(WindowManagerTests, DisabledInit)
{
  windowManagerAddWindow(mockedWindow, FALSE);  
  MockedWindow* data = (MockedWindow*)windowGetInternalData(mockedWindow);
  EXPECT_EQ(data->initializeRefCount, 0);
}

TEST_F(WindowManagerTests, UpdateTriggered)
{
  windowManagerAddWindow(mockedWindow, TRUE);
  windowManagerUpdate(0.0f);
  MockedWindow* data = (MockedWindow*)windowGetInternalData(mockedWindow);
  EXPECT_EQ(data->updateRefCount, 1);
}

TEST_F(WindowManagerTests, ProcessInputIsTriggered)
{
  windowManagerAddWindow(mockedWindow, TRUE);
  windowManagerProcessInput(EventData{}, nullptr);
  MockedWindow* data = (MockedWindow*)windowGetInternalData(mockedWindow);
  EXPECT_EQ(data->processInputRefCount, 1);
}
