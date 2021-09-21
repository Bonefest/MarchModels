#pragma once

#include "defines.h"
#include "logging.h"

struct Application;

EDITOR_API bool8 initApplication(uint32 width, uint32 height, const char* applicationName);
EDITOR_API void shutdownApplication();
EDITOR_API void runApplication();


