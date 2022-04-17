/**
 * A simple system for an automatic generation of the collections of the material's textures.
 *
 * The current version gets a list of materials from assets manager.
 */

#pragma once

#include "image.h"

ENGINE_API bool8 initializeMAS();
ENGINE_API void shutdownMAS();

ENGINE_API void masUpdate();
ENGINE_API ImagePtr masGetAtlas(uint32 index = 0);

