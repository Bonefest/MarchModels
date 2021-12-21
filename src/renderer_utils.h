#pragma once

#include "defines.h"

ENGINE_API void pushViewport();
ENGINE_API bool8 popViewport();

ENGINE_API void pushBlendingEquation();
ENGINE_API bool8 popBlendingEquation();

ENGINE_API void pushBlendingFunction();
ENGINE_API bool8 popBlendingFunction();

ENGINE_API void pushBlending();
ENGINE_API bool8 popBlending();
