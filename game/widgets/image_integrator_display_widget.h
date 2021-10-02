#pragma once

#include <string>
#include <image_integrator.h>

#include "widget.h"

bool8 createImageIntegratorDisplayWidget(const std::string& identifier,
                                         ImageIntegrator* integrator,
                                         float32 maxFPS,
                                         Widget** outWidget);

void imageIntegratorDisplayWidgetSetMaxFPS(Widget* widget, float32 maxFPS);
float32 imageIntegratorDisplayWidgetGetMaxFPS(Widget* widget);

void imageIntegratorDisplayWidgetSetTime(Widget* widget, float32 time);
void imageIntegratorDisplayWidgetPause(Widget* widget);
