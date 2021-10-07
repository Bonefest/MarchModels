#include <memory_manager.h>

#include "text_edit_widget.h"

const static uint32 MAX_BUF_SIZE = 2048;

struct TextEditWidgetData
{
  std::string identifier;
  char buffer[MAX_BUF_SIZE];
  
};

static bool8 textEditWidgetInitialize(Widget* widget)
{
  
  return TRUE;
}

static void textEditWidgetUpdate(Widget* widget, View* view, float64 delta)
{
  
}

static void textEditWidgetDraw(Widget* widget, View* view, float64 delta)
{
  TextEditWidgetData* data = (TextEditWidgetData*)widgetGetInternalData(widget);
  
  ImGui::Begin(data->identifier.c_str());
  ImVec2 editWindowSize = ImGui::GetWindowContentAreaSize();


  int32 offsetPosX, clearButtonSize = 65.0f, offset = 10.0f;
  {
    offsetPosX = editWindowSize.x - clearButtonSize;
    ImGui::SameLine(offsetPosX);  
    ImGui::Button(ICON_KI_TIMES" Clear");
    clearButtonSize = ImGui::GetItemRectSize().x;
  }

  {
    int32 compileButtonSize = 78.0f;
    offsetPosX = editWindowSize.x - clearButtonSize - compileButtonSize - offset;
    ImGui::SameLine(offsetPosX);
    ImGui::Button(ICON_KI_WRENCH" Compile");
  }
  
  ImGui::InputTextMultiline("##", data->buffer, MAX_BUF_SIZE, editWindowSize);
  ImGui::End();
}

bool8 createTextEditWidget(const std::string& identifier, Widget** outWidget)
{
  WidgetInterface interface = {};
  interface.initialize = textEditWidgetInitialize;
  interface.update = textEditWidgetUpdate;
  interface.draw = textEditWidgetDraw;

  allocateWidget(interface, outWidget);

  TextEditWidgetData* data = engineAllocObject<TextEditWidgetData>(MEMORY_TYPE_GENERAL);
  data->identifier = identifier;

  widgetSetInternalData(*outWidget, data);

  return TRUE;
}

void textEditWidgetSetText(Widget* widget, const std::string& text)
{

}

const std::string& textEditWidgetGetText(Widget* widget)
{
  return "";
}
