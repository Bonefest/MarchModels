#include <deque>

#include <event_system.h>
#include <memory_manager.h>

#include "console_widget.h"

const static ImColor mapLogMessageTypeToColor[] =
{
  ImColor(192, 0, 0),
  ImColor(192, 192, 0),
  ImColor(192, 0, 192),
  ImColor(66, 135, 246),
};

struct LogMessage
{
  std::string message;
  ImColor color;
};

struct ConsoleWidgetData
{
  std::deque<LogMessage> messages;
};

bool8 logMessageCallback(EventData data, void* sender, void* listener)
{
  assert(data.type == EVENT_TYPE_LOG_MESSAGE);

  LogMessage logMessage = {};
  logMessage.message = (char*)data.ptr[0];
  logMessage.color = mapLogMessageTypeToColor[data.u32[0]];
  
  Widget* widget = (Widget*)listener;
  ConsoleWidgetData* widgetData = (ConsoleWidgetData*)widgetGetInternalData(widget);
  widgetData->messages.push_front(logMessage);
  
  return FALSE;
}

static bool8 consoleWidgetInitialize(Widget* widget)
{
  registerListener(EVENT_TYPE_LOG_MESSAGE, widget, logMessageCallback);
  return TRUE;
}
   
static void consoleWidgetShutdown(Widget* widget)
{
  ConsoleWidgetData* data = (ConsoleWidgetData*)widgetGetInternalData(widget);
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
  
  unregisterListener(EVENT_TYPE_LOG_MESSAGE, widget);
}
  
static void consoleWidgetUpdate(Widget* widget, View* view, float64 delta)
{

}

static void consoleWidgetDraw(Widget* widget, View* view, float64 delta)
{
  ConsoleWidgetData* data = (ConsoleWidgetData*)widgetGetInternalData(widget);
  
  char tempBuf[128];
  
  ImGui::Begin(widgetGetIdentifier(widget).c_str());
    ImVec2 windowSize = ImGui::GetWindowSize();

    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(192, 0, 0, 255));
      ImGui::Button(ICON_KI_TIMES);
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();    

    ImGui::SameLine();

    ImGui::InputText("##", tempBuf, 128);

    ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Header, (ImVec4)ImColor(32, 32, 32));    
      uint32 messageIdx = 0;
      for(const LogMessage& message: data->messages)
      {
        ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)message.color);
          ImGui::Selectable(message.message.c_str(), messageIdx % 2, ImGuiSelectableFlags_Disabled);
        ImGui::PopStyleColor();

        messageIdx++;
      }
    ImGui::PopStyleColor();      
    ImGui::PopStyleVar();
    
  ImGui::End();
}

static void consoleProcessInput(Widget* widget, View* view, const EventData& eventData, void* sender)
{

}

bool8 createConsoleWidget(const std::string& identifier, Widget** outWidget)
{
  WidgetInterface interface = {};
  interface.initialize = consoleWidgetInitialize;
  interface.shutdown = consoleWidgetShutdown;
  interface.update = consoleWidgetUpdate;
  interface.draw = consoleWidgetDraw;
  interface.processInput = consoleProcessInput;

  if(allocateWidget(interface, identifier, outWidget) == FALSE)
  {
    return TRUE;
  }

  ConsoleWidgetData* data = engineAllocObject<ConsoleWidgetData>(MEMORY_TYPE_GENERAL);
  widgetSetInternalData(*outWidget, data);
  
  return TRUE;
}

