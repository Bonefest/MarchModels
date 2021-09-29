#pragma once

#include "widget.h"

bool8 createTextEditWidget(Widget** outWidget);

void textEditWidgetSetText(Widget* widget, const std::string& text);
const std::string& textEditWidgetGetText(Widget* widget);
