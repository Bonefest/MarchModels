#pragma once

#include <string>

#include <defines.h>
#include <imgui/imgui.h>
#include <maths/common.h>
#include <event_system.h>

struct View;

struct ViewInterface
{
  bool8 (*initialize)(View*);
  void (*shutdown)(View*);

  void (*onLoad)(View*);
  void (*onUnload)(View*);
  void (*onResize)(View*, uint2 newViewSize);

  void (*updateLayout)(View*, uint2 newViewSize);
  void (*update)(View*, float64 delta);  
  void (*draw)(View*, float64 delta);
  void (*processInput)(View*, const EventData& eventData, void* sender);
};

bool8 createView(const std::string& name, const ViewInterface& interface, uint2 initialViewSize, View** outView);
void destroyView(View* view);

bool8 initializeView(View* view);

void viewOnLoad(View* view);
void viewOnUnload(View* view);
void viewOnResize(View* view, uint2 newViewSize);

void drawView(View* view, float64 delta);
void updateView(View* view, float64 delta);
void processInputView(View* view, const EventData& eventData, void* sender);

ImGuiID viewGetMainNodeID(View* view);
std::string viewGetName(View* view);

void viewSetInternalData(View* view, void* internalData);
void* viewGetInternalData(View* view);
