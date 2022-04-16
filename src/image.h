#pragma once

#include "ptr.h"
#include "defines.h"
#include "maths/common.h"

struct Image;

ENGINE_API bool8 createImage(GLuint texture, uint32 width, uint32 height, Image** outImage);
ENGINE_API void destroyImage(Image* image);

ENGINE_API uint32 imageGetWidth(Image* image);
ENGINE_API uint32 imageGetHeight(Image* image);
ENGINE_API uint2 imageGetSize(Image* image);

ENGINE_API void imageSetName(Image* image, const std::string& name);
ENGINE_API const std::string& imageGetName(Image* image);

ENGINE_API GLuint imageGetGLHandle(Image* image);

using ImagePtr = SharedPtr<Image, destroyImage>;
