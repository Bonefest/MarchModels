#pragma once

#include "defines.h"
#include "logging.h"

struct Application;

bool8 startApplication();

EDITOR_API uint32 applicationGetScreenWidth();
EDITOR_API uint32 applicationGetScreenHeight();
EDITOR_API const char* applicationGetName();
EDITOR_API GLFWwindow* applicationGetWindow();

