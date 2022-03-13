#pragma once

#include "image.h"
#include "defines.h"

ENGINE_API bool8 initializeImageManager();
ENGINE_API void shutdownImageManager();

ENGINE_API ImagePtr imageManagerLoadImage(const char* path);

ENGINE_API bool8 imageManagerHasImage(const char* path);

ENGINE_API bool8 imageManagerFreeImage(const char* path);
ENGINE_API void imageManagerFreeImages();

