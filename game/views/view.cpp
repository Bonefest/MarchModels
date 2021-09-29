#include <logging.h>
#include <memory_manager.h>

#include "view.h"


static const ImGuiWindowFlags viewWindowFlags =
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoCollapse |
  ImGuiWindowFlags_NoBackground |
  ImGuiWindowFlags_NoSavedSettings |
  ImGuiWindowFlags_NoBringToFrontOnFocus |
  ImGuiWindowFlags_NoDocking |
  ImGuiWindowFlags_NoNavFocus;

struct View
{
  ViewInterface interface;
  std::string name;

  ImVec2 viewSize;
  
  bool8 initialized;
  void* internalData;
};

bool8 createView(const std::string& name, const ViewInterface& interface, View** outView)
{
  *outView = engineAllocObject<View>(MEMORY_TYPE_GENERAL);
  View* view = *outView;
  view->interface = interface;
  view->name = name;  
  view->initialized = FALSE;
  view->internalData = nullptr;

  return TRUE;
}

void destroyView(View* view)
{
  if(view->initialized)
  {
    view->interface.shutdown(view);
  }

  engineFreeObject(view, MEMORY_TYPE_GENERAL);
}

bool8 initializeView(View* view)
{
  if(view->initialized)
  {
    LOG_ERROR("View cannot be initialized twice!");
    return FALSE;
  }

  view->initialized = view->interface.initialize(view);
  return view->initialized;
}

void viewOnLoad(View* view)
{
  view->interface.onLoad(view);
}

void viewOnUnload(View* view)
{
  view->interface.onUnload(view);
}

void drawView(View* view, ImVec2 viewOffset, ImVec2 viewSize, float64 delta)
{
  if(view->viewSize.x != viewSize.x || view->viewSize.y != viewSize.y)
  {
    view->interface.updateLayout(view, viewSize);
    view->viewSize = viewSize;
  }

  ImGuiID viewNodeID = viewGetMainNodeID(view);
  
  ImGui::SetNextWindowPos(viewOffset);
  ImGui::SetNextWindowSize(viewSize);
  
  ImGui::Begin(view->name.c_str(), nullptr, viewWindowFlags);
  ImGui::DockSpace(viewNodeID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);  
  ImGui::End();
  
  view->interface.draw(view, viewOffset, viewSize, delta);
}

void updateView(View* view, float64 delta)
{
  view->interface.update(view, delta);
}

void processInputView(View* view, const EventData& eventData, void* sender)
{
  view->interface.processInput(view, eventData, sender);
}

ImGuiID viewGetMainNodeID(View* view)
{
  return ImGui::GetID(view->name.c_str());
}
  
std::string viewGetName(View* view)
{
  return view->name;
}

void viewSetInternalData(View* view, void* internalData)
{
  view->internalData = internalData;
}

void* viewGetInternalData(View* view)
{
  return view->internalData;
}
