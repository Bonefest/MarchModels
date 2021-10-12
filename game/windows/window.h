#pragma once

#include <string>

#include <defines.h>
#include <event_system.h>

struct Window;

struct WindowInterface
{
  bool8 (*initialize)(Window*);
  void (*shutdown)(Window*);
  void (*update)(Window* window, float64 delta);
  void (*draw)(Window* window, float64 delta);
  void (*processInput)(Window* window, const EventData& eventData, void* sender);
};

bool8 allocateWindow(WindowInterface interface, const std::string& identifier, Window** outWindow);
void freeWindow(Window* window);

bool8 initializeWindow(Window* window);
void updateWindow(Window* window, float64 delta);
void drawWindow(Window* window, float64 delta);
void processInputWindow(Window* window, const EventData& eventData, void* sender);

const std::string& windowGetIdentifier(Window* window);

void windowSetInternalData(Window* window, void* internalData);
void* windowGetInternalData(Window* window);
