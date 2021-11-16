#include <memory_manager.h>

#include "ui_styles.h"
#include "list_window.h"

using std::string;
using std::vector;

struct ListWindowData
{
  string tip;
  vector<ListItem> items;

  fpOnListItemSelected selectCb;
  void* cbUserData;

  string marker;
  bool8 closeOnLoseFocus;
  bool8 showIndex;
};

static bool8 listWindowInitialize(Window* window);
static void listWindowShutdown(Window* window);
static void listWindowUpdate(Window* window, float64 delta);
static void listWindowDraw(Window* window, float64 delta);
static void listWindowProcessInput(Window* window, const EventData& eventData, void* sender);
  
bool8 createListWindow(const string& identifier,
                       const string& tip,                       
                       const vector<ListItem>& items,
                       fpOnListItemSelected selectCallback,
                       void* userData,
                       Window** outWindow)
                       
{

  WindowInterface interface = {};
  interface.initialize = listWindowInitialize;
  interface.shutdown = listWindowShutdown;
  interface.update = listWindowUpdate;
  interface.draw = listWindowDraw;
  interface.processInput = listWindowProcessInput;
  
  if(allocateWindow(interface, identifier, outWindow) == FALSE)
  {
    return FALSE;
  }

  ListWindowData* data = engineAllocObject<ListWindowData>(MEMORY_TYPE_GENERAL);
  data->tip = tip;
  data->items = items;
  data->selectCb = selectCallback;
  data->cbUserData = userData;

  windowSetInternalData(*outWindow, data);
  
  return TRUE;
}

void listWindowSetCloseOnLoseFocus(Window* window, bool8 close)
{
  ListWindowData* data = (ListWindowData*)windowGetInternalData(window);
  data->closeOnLoseFocus = close;
}

void listWindowSetShowIndex(Window* window, bool8 show)
{
  ListWindowData* data = (ListWindowData*)windowGetInternalData(window);
  data->showIndex = show;
}

void listWindowSetMarker(Window* window, const std::string& marker)
{
  ListWindowData* data = (ListWindowData*)windowGetInternalData(window);  
  data->marker = marker;
}

bool8 listWindowInitialize(Window* window)
{
  ImGuiWindowFlags windowFlags = windowGetFlags(window);

  if((windowFlags & ImGuiWindowFlags_NoTitleBar) == 0)
  {
    windowFlags |= ImGuiWindowFlags_NoTitleBar;
  }

  if((windowFlags & ImGuiWindowFlags_NoResize) == 0)
  {
    windowFlags |= ImGuiWindowFlags_NoResize;
  }

  windowSetFlags(window, windowFlags);
  
  windowSetStyle(window, ImGuiStyleVar_WindowRounding, 0.0f);
  windowSetStyle(window, ImGuiStyleVar_WindowBorderSize, 0.5f);
  
  return TRUE;
}

void listWindowShutdown(Window* window)
{
  ListWindowData* data = (ListWindowData*)windowGetInternalData(window);
  if(data != nullptr)
  {
    engineFreeObject(data, MEMORY_TYPE_GENERAL);
  }
}

void listWindowUpdate(Window* window, float64 delta)
{

}

void listWindowDraw(Window* window, float64 delta)
{
  ListWindowData* data = (ListWindowData*)windowGetInternalData(window);  
  if(data->closeOnLoseFocus == TRUE && windowIsFocused(window) == FALSE)
  {
    windowClose(window);
    return;
  }

  if(data->tip.size() > 0)
  {
    ImGui::PushStyleColor(ImGuiCol_Text, (float4)HighlightSecondaryClr);
      ImGui::Text("%s", data->tip.c_str());
    ImGui::PopStyleColor();
    ImGui::Spacing();
  }
  
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
  uint32 itemIdx = 0;
  for(const ListItem& item: data->items)
  {
    if(ImGui::Selectable(item.label.c_str(), false))
    {
      data->selectCb(window, item.data, itemIdx, data->cbUserData);
      windowClose(window);
    }

    ImGui::SameLine();
    ImGui::Text(ICON_KI_CARET_RIGHT);

    itemIdx++;
  }
  ImGui::PopStyleVar();
  
}

void listWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{

}
