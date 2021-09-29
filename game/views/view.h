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

  void (*updateLayout)(View*, ImVec2 newViewSize);
  void (*update)(View*, float64 delta);  
  void (*draw)(View*, ImVec2 viewOffset, ImVec2 viewSize, float64 delta);
  void (*processInput)(View*, const EventData& eventData, void* sender);
};

bool8 createView(const std::string& name, const ViewInterface& interface, View** outView);
void destroyView(View* view);

bool8 initializeView(View* view);

void viewOnLoad(View* view);
void viewOnUnload(View* view);
void viewOnResize(View* view, uint2 newViewSize);

void drawView(View* view, ImVec2 viewOffset, ImVec2 viewSize, float64 delta);
void updateView(View* view, float64 delta);
void processInputView(View* view, const EventData& eventData, void* sender);

ImGuiID viewGetMainNodeID(View* view);
std::string viewGetName(View* view);

void viewSetInternalData(View* view, void* internalData);
void* viewGetInternalData(View* view);
