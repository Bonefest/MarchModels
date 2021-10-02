#pragma once

#include "widget.h"

bool8 createTextEditWidget(const std::string& identifier, Widget** outWidget);

void textEditWidgetSetText(Widget* widget, const std::string& text);
const std::string& textEditWidgetGetText(Widget* widget);
