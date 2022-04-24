#pragma once

#include "defines.h"
#include "logging.h"

struct Application;

bool8 startApplication();

ENGINE_API uint32 applicationGetScreenWidth();
ENGINE_API uint32 applicationGetScreenHeight();
ENGINE_API const char* applicationGetName();
ENGINE_API GLFWwindow* applicationGetWindow();

ENGINE_API void applicationSetFPS(uint32 FPS);
ENGINE_API uint32 applicationGetFPS();

// Defines whether application behaves as it has given FPS even
// if it's slower in reality
ENGINE_API void applicationSetFPSFixed(bool8 fixed);
ENGINE_API bool8 applicationIsFPSFixed();
