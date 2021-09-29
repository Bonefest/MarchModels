#pragma once

#include <string>

#include "defines.h"
#include "views/view.h"

void editorAddView(View* view);
bool8 editorSetView(const std::string& viewName);
View* editorGetCurrentView();
