#pragma once

#include <string>
#include <image_integrator.h>

#include "window.h"

bool8 createViewWindow(const std::string& identifier,
                                         ImageIntegrator* integrator,
                                         float32 maxFPS,
                                         Window** outWindow);

void viewWindowSetMaxFPS(Window* window, float32 maxFPS);
float32 viewWindowGetMaxFPS(Window* window);

void viewWindowSetTime(Window* window, float32 time);
void viewWindowPause(Window* window);


