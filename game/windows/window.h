#pragma once

#include <string>

#include <imgui/imgui.h>

#include <defines.h>
#include <event_system.h>
#include <maths/common.h>

struct Window;

struct WindowInterface
{
  bool8 (*initialize)(Window* window);
  void (*shutdown)(Window* window);
  void (*update)(Window* window, float64 delta);
  void (*draw)(Window* window, float64 delta);
  void (*processInput)(Window* window, const EventData& eventData, void* sender);

  bool8 usesCustomDrawPipeline = FALSE;
};

bool8 allocateWindow(WindowInterface interface, const std::string& identifier, Window** outWindow);
void freeWindow(Window* window);

bool8 initWindow(Window* window);
void updateWindow(Window* window, float64 delta);
void drawWindow(Window* window, float64 delta);
void processInputWindow(Window* window, const EventData& eventData, void* sender);

void windowSetSize(Window* window, float2 size);
float2 windowGetSize(Window* window);

void windowSetPosition(Window* window, float2 position);
float2 windowGetPosition(Window* window);

void windowSetOpen(Window* window, bool8 open);
bool8 windowIsOpen(Window* window);

void windowSetVisible(Window* window, bool8 visible);
bool8 windowIsVisible(Window* window);

void windowSetFocused(Window* window, bool8 focused);
bool8 windowIsFocused(Window* window);

void windowSetCollapsed(Window* window, bool8 collapsed);
bool8 windowIsCollapsed(Window* window);

void windowSetFlags(Window* window, ImGuiWindowFlags flags);
ImGuiWindowFlags windowGetFlags(Window* window);

const std::string& windowGetIdentifier(Window* window);

void windowSetInternalData(Window* window, void* internalData);
void* windowGetInternalData(Window* window);
