#include <list>
#include <deque>

#include <utils.h>
#include <logging.h>
#include <cvar_system.h>
#include <imgui/imgui.h>
#include <event_system.h>
#include <memory_manager.h>

#include "ui_styles.h"
#include "console_window.h"

const static uint32 MAX_BUF_SIZE = 255;


struct LogMessageData
{
  LogMessageType type;  
  std::string message;
  ImColor color;
};

struct ConsoleWindowData
{
  std::deque<LogMessageData> messages;
  std::vector<std::string> history;
  int32 historyIdx = 0;
  
  char textBuffer[MAX_BUF_SIZE]{};

  uint8 filterTypes = 0xFF;
};

bool8 logMessageCallback(EventData data, void* sender, void* listener)
{
  assert(data.type == EVENT_TYPE_LOG_MESSAGE);

  LogMessageData logMessage = {};
  logMessage.type = (LogMessageType)data.u32[0];
  logMessage.message = (char*)data.ptr[0];
  logMessage.color = logTypeToClr((LogMessageType)data.u32[0]);
  
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
    else if(data->EventKey == ImGuiKey_DownArrow)
    {
      windowData->historyIdx = (windowData->historyIdx + historySize - 1) % historySize;
    }

    std::string& message = windowData->history[windowData->historyIdx];
    
    strcpy(data->Buf, message.c_str());
    data->BufTextLen = strlen(data->Buf);
    data->CursorPos = data->BufTextLen;
    data->BufDirty = true;
  }
  else if((data->EventFlag & ImGuiInputTextFlags_CallbackCompletion) == ImGuiInputTextFlags_CallbackCompletion)
  {
    if(data->EventKey == ImGuiKey_Tab)
    {
      if(data->Buf[0] != '/')
      {
        return 0;
      }
      
      std::string targetPrefix = data->Buf + 1;
      std::string closestMatch = "";
      
      std::vector<std::string> cvarNames = CVarSystemGetRegisteredVars();

      for(std::string name: cvarNames)
      {
        if(targetPrefix == name.substr(0, targetPrefix.size()))
        {
          if(closestMatch.empty())
          {
            closestMatch = name;
          }
          else
          {
            // Find the first character where current closest match is different from given name
            uint32 diffPos = targetPrefix.size();
            for(; diffPos < std::min(closestMatch.size(), name.size()); diffPos++)
            {
              if(name[diffPos] != closestMatch[diffPos])
              {
                break;
              }
            }

            closestMatch = closestMatch.substr(0, diffPos);
          }
        }
      }
      
      if(!closestMatch.empty())
      {
        sprintf(data->Buf, "/%s", closestMatch.c_str());
        data->BufTextLen = strlen(data->Buf);
        data->CursorPos = data->BufTextLen;
        data->BufDirty = true;        
      }
       
    }

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

  float2 windowSize = ImGui::GetWindowSize();
  ImGuiStyle& style = ImGui::GetStyle();

  // Rendering helper elements
  float32 hItemSpace = 2;
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, float2(hItemSpace, style.ItemSpacing.y));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

    const char* items[] = {"error", "warning", "verbose", "info", "success"};
    if(ImGui::BeginCombo("##Console_filter_combo", "", ImGuiComboFlags_NoPreview))
    {
      if(ImGui::Selectable("none", false))
      {
        data->filterTypes = 0;
      }
      
      for(uint32 i = 0; i < (uint32)ARRAY_SIZE(items); i++)
      {
        if(ImGui::Selectable(items[i], (data->filterTypes & (1 << i)) == (1 << i)))
        {
          data->filterTypes ^= (1 << i);
        }
      }

      if(ImGui::Selectable("all", false))
      {
        data->filterTypes = 0xFF;
      }

      ImGui::EndCombo();
    }

    float32 comboWidth = ImGui::GetItemRectSize().x;

    ImGui::SameLine();

    float32 inputTextWidth = windowSize.x - comboWidth - hItemSpace * 2 - style.ScrollbarSize - style.WindowPadding.x;

    ImGui::SetNextItemWidth(inputTextWidth);
    const ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue |
      ImGuiInputTextFlags_CallbackHistory |
      ImGuiInputTextFlags_CallbackCompletion;
    
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
        triggerEvent(eventData, window);

        // If command is written
        if(data->textBuffer[0] == '/')
        {
          std::string command = (data->textBuffer + 1);
          auto spacePos = command.find(' ');

          // Reading cvar value
          if(spacePos == std::string::npos)
          {
            std::string cvarValue = CVarSystemReadStr(command);
            if(!cvarValue.empty())
            {
              LOG_INFO("%s = %s", command.c_str(), cvarValue.c_str());
            }
            else
            {
              LOG_ERROR("Var '%s' is not found!", command.c_str());
            }
          }
          // Writing cvar value
          else
          {
            std::string varName = command.substr(0, spacePos);
            std::string params = command.substr(spacePos + 1);

            CVarParseCode code = CVarSystemParseStr(varName, params);
            switch(code)
            {
              case CVAR_PARSE_CODE_SUCCESS: LOG_SUCCESS("%s = %s", varName.c_str(), params.c_str()); break;
              case CVAR_PARSE_CODE_VAR_NOT_FOUND: LOG_ERROR("Var '%s' is not found!", varName.c_str()); break;
              case CVAR_PARSE_CODE_VAR_READ_ONLY: LOG_ERROR("Var '%s' is read only!", varName.c_str()); break;
              case CVAR_PARSE_CODE_CANNOT_PARSE: LOG_ERROR("Cannot parse '%s'!", params.c_str()); break;

              default: assert(false);
            }
          }
        }
        
        // TODO: In future we may want to add only successful messages
        // TODO: insert is too costly operation!
        data->history.insert(data->history.begin(), data->textBuffer);
        data->historyIdx = 0;

        data->textBuffer[0] = '\0';
      }
      else
      {
        LOG_INFO(data->textBuffer);        
      }
    }

  ImGui::PopStyleVar(2);

  // Rendering list of messages
  float2 avalReg = ImGui::GetContentRegionAvail();
  
  ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);
  ImGui::PushStyleColor(ImGuiCol_Header, (float4)ImColor(32, 32, 32, 255));    
    uint32 messageIdx = 1;
    for(const LogMessageData& message: data->messages)
    {
      if( (data->filterTypes & (1 << (uint8)message.type)) != (1 << (uint8)message.type) )
      {
        continue;
      }

      float2 cursorPosBeforeWc = ImGui::GetCursorPos();
      float2 cursorPosBefore = ImGui::GetCursorScreenPos();
      
      ImGui::PushStyleColor(ImGuiCol_Text, (float4)InvisibleClr);
        ImGui::TextWrapped("%s", message.message.c_str());
      ImGui::PopStyleColor();
      
      float2 cursorPosAfter = ImGui::GetCursorScreenPos();

      ImGui::GetWindowDrawList()->AddRectFilled(cursorPosBefore,
                                                float2(cursorPosBefore.x + avalReg.x, cursorPosAfter.y),
                                                messageIdx % 2 == 0 ? ImColor(20, 20, 35, 255) : ImColor(10, 10, 10, 255));

      ImGui::SetCursorPos(cursorPosBeforeWc);
      
      ImGui::PushStyleColor(ImGuiCol_Text, (float4)message.color);
        ImGui::TextWrapped("%s", message.message.c_str());
      ImGui::PopStyleColor();

      messageIdx++;
    }
  ImGui::PopStyleColor();      
  ImGui::PopStyleVar();
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

