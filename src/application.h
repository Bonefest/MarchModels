#pragma once

#include "defines.h"
#include "logging.h"

struct Application;

bool8 startApplication();

ENGINE_API uint32 applicationGetScreenWidth();
ENGINE_API uint32 applicationGetScreenHeight();
ENGINE_API const char* applicationGetName();
ENGINE_API GLFWwindow* applicationGetWindow();

