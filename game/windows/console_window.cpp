#include <list>
#include <deque>

#include <logging.h>
#include <imgui/imgui.h>
#include <event_system.h>
#include <memory_manager.h>

#include "console_window.h"

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

struct ConsoleWindowData
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
  
  Window* window = (Window*)listener;
  ConsoleWindowData* windowData = (ConsoleWindowData*)windowGetInternalData(window);
  windowData->messages.push_front(logMessage);
  
  return FALSE;
}

int inputTextCallback(ImGuiInputTextCallbackData* data)
{
  Window* consoleWindow = (Window*)data->UserData;
  ConsoleWindowData* windowData = (ConsoleWindowData*)windowGetInternalData(consoleWindow);
  
  if((data->EventFlag & ImGuiInputTextFlags_CallbackHistory) == ImGuiInputTextFlags_CallbackHistory
     && !windowData->history.empty())
  {
    uint32 historySize = windowData->history.size();
    
    if(data->EventKey == ImGuiKey_UpArrow)
    {
      windowData->historyIdx = (windowData->historyIdx + 1) % historySize;
    }
    else
    {
      windowData->historyIdx = (windowData->historyIdx + historySize - 1) % historySize;
    }

    std::string& message = windowData->history[windowData->historyIdx];
    
    strcpy(data->Buf, message.c_str());
    data->BufTextLen = strlen(data->Buf);
    data->CursorPos = data->BufTextLen;
    data->BufDirty = true;
  }

  return 0;
}

static bool8 consoleWindowInitialize(Window* window)
{
  registerListener(EVENT_TYPE_LOG_MESSAGE, window, logMessageCallback);
  return TRUE;

}
   
static void consoleWindowShutdown(Window* window)
{
  ConsoleWindowData* data = (ConsoleWindowData*)windowGetInternalData(window);
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
  
  unregisterListener(EVENT_TYPE_LOG_MESSAGE, window);
}
  
static void consoleWindowUpdate(Window* window, float64 delta)
{

}

static void consoleWindowDraw(Window* window, float64 delta)
{
  ConsoleWindowData* data = (ConsoleWindowData*)windowGetInternalData(window);
  
  ImGui::Begin(windowGetIdentifier(window).c_str());
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
                          window))
      {
        EventData eventData = {};
        eventData.u32[0] = strlen(data->textBuffer);
        eventData.ptr[0] = data->textBuffer;

        if(eventData.u32[0] > 0)
        {
          triggerEvent(EVENT_TYPE_CONSOLE_MESSAGE, eventData, window);
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

static void consoleProcessInput(Window* window, const EventData& eventData, void* sender)
{

}

bool8 createConsoleWindow(const std::string& identifier, Window** outWindow)
{
  WindowInterface interface = {};
  interface.initialize = consoleWindowInitialize;
  interface.shutdown = consoleWindowShutdown;
  interface.update = consoleWindowUpdate;
  interface.draw = consoleWindowDraw;
  interface.processInput = consoleProcessInput;

  if(allocateWindow(interface, identifier, outWindow) == FALSE)
  {
    return TRUE;
  }

  ConsoleWindowData* data = engineAllocObject<ConsoleWindowData>(MEMORY_TYPE_GENERAL);
  windowSetInternalData(*outWindow, data);
  
  return TRUE;
}

