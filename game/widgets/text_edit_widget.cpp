#include "text_edit_widget.h"

static bool8 textEditWidgetInitialize(Widget* widget)
{
  
  return TRUE;
}

static void textEditWidgetUpdate(Widget* widget, View* view, float64 delta)
{
  
}

static void textEditWidgetDraw(Widget* widget, View* view, float64 delta)
{
  char buf[255];
  
  ImGui::Begin("Text edit widget");
  ImGui::InputTextMultiline("SDF Script", buf, 255);
  ImGui::End();
}

bool8 createTextEditWidget(Widget** outWidget)
{
  WidgetInterface interface = {};
  interface.initialize = textEditWidgetInitialize;
  interface.update = textEditWidgetUpdate;
  interface.draw = textEditWidgetDraw;

  return allocateWidget(interface, outWidget);
}

void textEditWidgetSetText(Widget* widget, const std::string& text)
{

}

const std::string& textEditWidgetGetText(Widget* widget)
{
  return "";
}
