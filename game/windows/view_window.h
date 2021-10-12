#pragma once

#include <string>
#include <image_integrator.h>

#include "window.h"

/**
 * @warning: View window manages lifetime of given objects itself.
 */
bool8 createViewWindow(const std::string& identifier,
                       Sampler* sampler,
                       RayIntegrator* rayIntegrator,
                       Camera* camera,
                       Window** outWindow);

void viewWindowSetMaxFPS(Window* window, float32 maxFPS);
float32 viewWindowGetMaxFPS(Window* window);

void viewWindowSetTime(Window* window, float32 time);
void viewWindowPause(Window* window);


