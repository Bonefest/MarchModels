#include <list>
#include <deque>

#include <logging.h>
#include <event_system.h>
#include <memory_manager.h>

#include "console_widget.h"

const static uint32 MAX_BUF_SIZE = 255;

const static ImColor mapLogMessageTypeToColor[] =
{
  ImColor(192, 0, 0),
  ImColor(192, 192, 0),
  ImColor(192, 0, 192),
  ImColor(66, 135, 246),
};

struct LogMessage
{
  LogMessageType type;  
  std::string message;
  ImColor color;
};

struct ConsoleWidgetData
{
  std::deque<LogMessage> messages;
  std::vector<std::string> history;
  int32 historyIdx = 0;
  
  char textBuffer[MAX_BUF_SIZE]{};
  
  uint32 filterType = (uint32)LOG_MESSAGE_TYPE_COUNT;
};

bool8 logMessageCallback(EventData data, void* sender, void* listener)
{
  assert(data.type == EVENT_TYPE_LOG_MESSAGE);

  LogMessage logMessage = {};
  logMessage.type = (LogMessageType)data.u32[0];
  logMessage.message = (char*)data.ptr[0];
  logMessage.color = mapLogMessageTypeToColor[data.u32[0]];
  
  Widget* widget = (Widget*)listener;
  ConsoleWidgetData* widgetData = (ConsoleWidgetData*)widgetGetInternalData(widget);
  widgetData->messages.push_front(logMessage);
  
  return FALSE;
}

int inputTextCallback(ImGuiInputTextCallbackData* data)
{
  Widget* consoleWidget = (Widget*)data->UserData;
  ConsoleWidgetData* widgetData = (ConsoleWidgetData*)widgetGetInternalData(consoleWidget);
  
  if((data->EventFlag & ImGuiInputTextFlags_CallbackHistory) == ImGuiInputTextFlags_CallbackHistory
     && !widgetData->history.empty())
  {
    uint32 historySize = widgetData->history.size();
    
    if(data->EventKey == ImGuiKey_UpArrow)
    {
      widgetData->historyIdx = (widgetData->historyIdx + 1) % historySize;
    }
    else
    {
      widgetData->historyIdx = (widgetData->historyIdx + historySize - 1) % historySize;
    }

    std::string& message = widgetData->history[widgetData->historyIdx];
    
    strcpy(data->Buf, message.c_str());
    data->BufTextLen = strlen(data->Buf);
    data->CursorPos = data->BufTextLen;
    data->BufDirty = true;
  }

  return 0;
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
  
  ImGui::Begin(widgetGetIdentifier(widget).c_str());
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImGuiStyle& style = ImGui::GetStyle();

    // Rendering helper elements
    float32 hItemSpace = 2;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(hItemSpace, style.ItemSpacing.y));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    
      const char* items[] = {"error", "warning", "verbose", "info", "all"};
      if(ImGui::BeginCombo("##Console_filter_combo", "", ImGuiComboFlags_NoPreview))
      {
        for(uint32 i = 0; i <= (uint32)LOG_MESSAGE_TYPE_COUNT; i++)
        {
          if(ImGui::Selectable(items[i], data->filterType == i))
          {
            data->filterType = i;
          }
        }

        ImGui::EndCombo();
      }

      float32 comboWidth = ImGui::GetItemRectSize().x;
      
      ImGui::SameLine();

      float32 inputTextWidth = windowSize.x - comboWidth - hItemSpace * 2 - style.ScrollbarSize - style.WindowPadding.x;

      ImGui::SetNextItemWidth(inputTextWidth);
      const ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue |
        ImGuiInputTextFlags_CallbackHistory;
      if(ImGui::InputText("##Console_search_text",
                          data->textBuffer,
                          MAX_BUF_SIZE,
                          inputTextFlags,
                          inputTextCallback,
                          widget))
      {
        EventData eventData = {};
        eventData.u32[0] = strlen(data->textBuffer);
        eventData.ptr[0] = data->textBuffer;

        if(eventData.u32[0] > 0)
        {
          triggerEvent(EVENT_TYPE_CONSOLE_MESSAGE, eventData, widget);
          LOG_INFO(data->textBuffer);

          // TODO: In future we may want to add only successful messages
          // TODO: insert is too costly operation!
          data->history.insert(data->history.begin(), data->textBuffer);
          data->historyIdx = 0;
          
          data->textBuffer[0] = '\0';
        }
      }

    ImGui::PopStyleVar(2);

    // Rendering list of messages
    ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Header, (ImVec4)ImColor(32, 32, 32));    
      uint32 messageIdx = 1;
      for(const LogMessage& message: data->messages)
      {
        if((uint32)message.type != data->filterType && data->filterType != (uint32)LOG_MESSAGE_TYPE_COUNT)
        {
          continue;
        }
        
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

