#pragma once

#include "defines.h"
#include "event_system.h"

struct Application;

struct GameFramework
{
  bool8 (*extractSetupConfig)(Application* app,
                              uint32* outScreenWidth,
                              uint32* outScreenHeight,
                              const char** outName);
  bool8 (*initialize)(Application* app);
  void (*shutdown)(Application* app);
  void (*update)(Application* app, float64 delta);
  void (*draw)(Application* app, float64 delta);
  void (*processInput)(Application* app, const EventData& eventData, void* sender);
};

/** To be implemented by a user */
bool8 initializeGameFramework(GameFramework* outFramework);

